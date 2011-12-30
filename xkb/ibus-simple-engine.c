/* -*- mode: C; c-basic-offset: 4; indent-tabs-mode: nil; -*- */
/* vim:set et sts=4: */
#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include "ibus-simple-engine.h"

#include <glib/gi18n-lib.h>
#include <stdlib.h>
#include <memory.h>

#ifdef ENABLE_NLS
#include <locale.h>
#endif

#define MAX_COMPOSE_LEN 7

typedef enum {
    IBUS_COMPOSE_TABLE_NONE             = 0,
    IBUS_COMPOSE_TABLE_CEDILLA,
} IBusComposeAddOnTableType;

typedef struct _IBusSimpleEngine IBusSimpleEngine;
typedef struct _IBusSimpleEngineClass IBusSimpleEngineClass;

struct _IBusSimpleEngine {
    IBusEngine parent;

    IBusPropList *prop_list;

    guint      compose_buffer[MAX_COMPOSE_LEN + 1];
    gunichar   tentative_match;
    gint       tentative_match_len;

    guint      in_hex_sequence : 1;
    guint      modifiers_dropped : 1;
};

struct _IBusSimpleEngineClass {
    IBusEngineClass parent;
    IBusComposeAddOnTableType compose_addon_table_type;
};

typedef struct _GtkComposeTableCompact GtkComposeTableCompact;
struct _GtkComposeTableCompact
{
    const guint16 *data;
    gint max_seq_len;
    gint n_index_size;
    gint n_index_stride;
};

/* This file contains the table of the compose sequences,
 * static const guint16 gtk_compose_seqs_compact[] = {}
 * IT is generated from the compose-parse.py script.
 */
#include "gtkimcontextsimpleseqs.h"

/* From the values below, the value 23 means the number of different first keysyms
 * that exist in the Compose file (from Xorg). When running compose-parse.py without
 * parameters, you get the count that you can put here. Needed when updating the
 * gtkimcontextsimpleseqs.h header file (contains the compose sequences).
 */
static const GtkComposeTableCompact gtk_compose_table_compact = {
    gtk_compose_seqs_compact,
    5,
    24,
    6
};

static const guint16 gtk_compose_ignore[] = {
    IBUS_Shift_L,
    IBUS_Shift_R,
    IBUS_Control_L,
    IBUS_Control_R,
    IBUS_Caps_Lock,
    IBUS_Shift_Lock,
    IBUS_Meta_L,
    IBUS_Meta_R,
    IBUS_Alt_L,
    IBUS_Alt_R,
    IBUS_Super_L,
    IBUS_Super_R,
    IBUS_Hyper_L,
    IBUS_Hyper_R,
    IBUS_Mode_switch,
    IBUS_ISO_Level3_Shift
};

/* Copied from gtk+2.0-2.20.1/modules/input/imcedilla.c to fix crosbug.com/11421.
 * Overwrite the original Gtk+'s compose table in gtk+-2.x.y/gtk/gtkimcontextsimple.c. */

/* The difference between this and the default input method is the handling
 * of C+acute - this method produces C WITH CEDILLA rather than C WITH ACUTE.
 * For languages that use CCedilla and not acute, this is the preferred mapping,
 * and is particularly important for pt_BR, where the us-intl keyboard is
 * used extensively.
 */
static guint16 cedilla_compose_seqs[] = {
  IBUS_dead_acute,      IBUS_C,   0,      0,      0,      0x00C7, /* LATIN_CAPITAL_LETTER_C_WITH_CEDILLA */
  IBUS_dead_acute,      IBUS_c,   0,      0,      0,      0x00E7, /* LATIN_SMALL_LETTER_C_WITH_CEDILLA */
  IBUS_Multi_key,       IBUS_apostrophe, IBUS_C,  0,      0,      0x00C7, /* LATIN_CAPITAL_LETTER_C_WITH_CEDILLA */
  IBUS_Multi_key,       IBUS_apostrophe, IBUS_c,  0,      0,      0x00E7, /* LATIN_SMALL_LETTER_C_WITH_CEDILLA */
  IBUS_Multi_key,       IBUS_C,  IBUS_apostrophe, 0,      0,      0x00C7, /* LATIN_CAPITAL_LETTER_C_WITH_CEDILLA */
  IBUS_Multi_key,       IBUS_c,  IBUS_apostrophe, 0,      0,      0x00E7, /* LATIN_SMALL_LETTER_C_WITH_CEDILLA */
};

/* functions prototype */
static void     ibus_simple_engine_class_init   (IBusSimpleEngineClass  *klass);
static void     ibus_simple_engine_init         (IBusSimpleEngine       *simple);
static void     ibus_simple_engine_destroy      (IBusSimpleEngine       *simple);
static void     ibus_simple_engine_reset        (IBusEngine             *engine);
static void     ibus_simple_engine_enable       (IBusEngine             *engine);
static void     ibus_simple_engine_disable      (IBusEngine             *engine);
static void     ibus_simple_engine_focus_in     (IBusEngine             *engine);
static void     ibus_simple_engine_focus_out    (IBusEngine             *engine);
static void     ibus_simple_engine_reset        (IBusEngine             *engine);
static void     ibus_simple_engine_property_activate
                                                (IBusEngine             *engine,
                                                 const gchar            *prop_name,
                                                 guint                   prop_state);
static gboolean ibus_simple_engine_process_key_event
                                                (IBusEngine             *engine,
                                                 guint                   keyval,
                                                 guint                   keycode,
                                                 guint                   modifiers);
static void     ibus_simple_engine_commit_char (IBusSimpleEngine        *simple,
                                                gunichar                 ch);
static void     ibus_simple_engine_update_preedit_text
                                               (IBusSimpleEngine        *simple);

G_DEFINE_TYPE (IBusSimpleEngine, ibus_simple_engine, IBUS_TYPE_ENGINE)

static void
ibus_simple_engine_class_init (IBusSimpleEngineClass *klass)
{
    IBusObjectClass *ibus_object_class = IBUS_OBJECT_CLASS (klass);
    IBusEngineClass *engine_class = IBUS_ENGINE_CLASS (klass);
    gchar *lang = NULL;

    ibus_object_class->destroy = (IBusObjectDestroyFunc) ibus_simple_engine_destroy;

    engine_class->reset     = ibus_simple_engine_reset;
    engine_class->enable    = ibus_simple_engine_enable;
    engine_class->disable   = ibus_simple_engine_disable;
    engine_class->focus_in  = ibus_simple_engine_focus_in;
    engine_class->focus_out = ibus_simple_engine_focus_out;
    engine_class->property_activate
                            = ibus_simple_engine_property_activate;
    engine_class->process_key_event
                            = ibus_simple_engine_process_key_event;

    klass->compose_addon_table_type = IBUS_COMPOSE_TABLE_NONE;
 
#ifdef ENABLE_NLS
    lang = g_strdup (setlocale (LC_CTYPE, NULL));
#endif
    if (lang == NULL) {
        lang = g_strdup (g_getenv ("LANG"));
    }
    if (lang && g_ascii_strncasecmp (lang, "pt_BR", strlen ("pt_BR")) == 0) {
        klass->compose_addon_table_type = IBUS_COMPOSE_TABLE_CEDILLA;
    }
    g_free (lang);
}

static void
ibus_simple_engine_init (IBusSimpleEngine *simple)
{
#ifdef ENABLE_SETUP_GUI
    IBusText *label;
    IBusText *tooltip;
    IBusProperty *prop;

    simple->prop_list = ibus_prop_list_new ();

    g_object_ref_sink (simple->prop_list);
    label = ibus_text_new_from_string (_("Setup"));
    tooltip = ibus_text_new_from_string (_("Configure XKB engine"));
    prop = ibus_property_new ("setup",
                              PROP_TYPE_NORMAL,
                              label,
                              "gtk-preferences",
                              tooltip,
                              TRUE, TRUE,
                              PROP_STATE_UNCHECKED,
                              NULL);
    g_object_ref_sink (prop);
    ibus_prop_list_append (simple->prop_list, prop);
#endif
}

static void
ibus_simple_engine_destroy (IBusSimpleEngine *simple)
{
#ifdef ENABLE_SETUP_GUI
    if (simple->prop_list) {
        g_object_unref (simple->prop_list);
        simple->prop_list = NULL;
    }
#endif

    ((IBusObjectClass *) ibus_simple_engine_parent_class)->destroy ((IBusObject *)simple);
}

static void
ibus_simple_engine_reset (IBusEngine *engine)
{
    IBusSimpleEngine *simple = (IBusSimpleEngine *)engine;
    simple->compose_buffer[0] = 0;

    if (simple->tentative_match || simple->in_hex_sequence) {
        simple->in_hex_sequence = FALSE;
        simple->tentative_match = 0;
        simple->tentative_match_len = 0;
        ibus_engine_hide_preedit_text ((IBusEngine *)simple);
    }
}

static void
ibus_simple_engine_enable (IBusEngine *engine)
{
    IBUS_ENGINE_CLASS (ibus_simple_engine_parent_class)->enable (engine);
}

static void
ibus_simple_engine_disable (IBusEngine *engine)
{
    IBUS_ENGINE_CLASS (ibus_simple_engine_parent_class)->disable (engine);
}

static void
ibus_simple_engine_focus_in (IBusEngine *engine)
{
#ifdef ENABLE_SETUP_GUI
    IBusSimpleEngine *simple = (IBusSimpleEngine *) engine;

    ibus_engine_register_properties (engine, simple->prop_list);
#endif
    IBUS_ENGINE_CLASS (ibus_simple_engine_parent_class)->focus_in (engine);
}

static void
ibus_simple_engine_focus_out (IBusEngine *engine)
{
    IBUS_ENGINE_CLASS (ibus_simple_engine_parent_class)->focus_out (engine);
}

static void
ibus_simple_engine_property_activate (IBusEngine  *engine,
                                      const gchar *prop_name,
                                      guint        prop_state)
{
    if (g_strcmp0 (prop_name, "setup") == 0) {
        gchar *argv[2] = { NULL, NULL};
        gchar *path;
        const gchar *libexecdir = g_getenv ("LIBEXECDIR");
        GError *error = NULL;

        if (libexecdir == NULL) {
            libexecdir = LIBEXECDIR;
        }
        path = g_build_filename (libexecdir, "ibus-setup-xkb", NULL);
        argv[0] = path;
        if (!g_spawn_async (NULL, argv, NULL, 0, NULL, NULL, NULL, &error)) {
            if (error) {
                g_warning ("exec failure: %s", error->message);
                g_error_free (error);
            }
        }
        g_free (path);
        return;
    }
    IBUS_ENGINE_CLASS (ibus_simple_engine_parent_class)->property_activate (engine, prop_name, prop_state);
}

static void
ibus_simple_engine_commit_char (IBusSimpleEngine *simple,
                                    gunichar             ch)
{
    g_return_if_fail (g_unichar_validate (ch));

    if (simple->tentative_match || simple->in_hex_sequence) {
        simple->in_hex_sequence = FALSE;
        simple->tentative_match = 0;
        simple->tentative_match_len = 0;
        ibus_simple_engine_update_preedit_text (simple);
    }

    ibus_engine_commit_text ((IBusEngine *)simple,
            ibus_text_new_from_unichar (ch));
}

static void
ibus_simple_engine_update_preedit_text (IBusSimpleEngine *simple)
{
    gunichar outbuf[MAX_COMPOSE_LEN + 2];
    int len = 0;

    if (simple->in_hex_sequence) {
        int hexchars = 0;

        outbuf[0] = L'u';
        len = 1;

        while (simple->compose_buffer[hexchars] != 0) {
            outbuf[len] = ibus_keyval_to_unicode (simple->compose_buffer[hexchars]);
            ++len;
            ++hexchars;
        }
        g_assert (len <= MAX_COMPOSE_LEN + 1);
    }
    else if (simple->tentative_match)
        outbuf[len++] = simple->tentative_match;

    outbuf[len] = L'\0';
    if (len == 0) {
        ibus_engine_hide_preedit_text ((IBusEngine *)simple);
    }
    else {
        IBusText *text = ibus_text_new_from_ucs4 (outbuf);
        ibus_text_append_attribute (text,
                IBUS_ATTR_TYPE_UNDERLINE, IBUS_ATTR_UNDERLINE_SINGLE, 0, len);
        g_debug ("UpdatePreedit text=%s", text->text);
        ibus_engine_update_preedit_text ((IBusEngine *)simple, text, len, TRUE);
    }
}


/* In addition to the table-driven sequences, we allow Unicode hex
 * codes to be entered. The method chosen here is similar to the
 * one recommended in ISO 14755, but not exactly the same, since we
 * don't want to steal 16 valuable key combinations.
 *
 * A hex Unicode sequence must be started with Ctrl-Shift-U, followed
 * by a sequence of hex digits entered with Ctrl-Shift still held.
 * Releasing one of the modifiers or pressing space while the modifiers
 * are still held commits the character. It is possible to erase
 * digits using backspace.
 *
 * As an extension to the above, we also allow to start the sequence
 * with Ctrl-Shift-U, then release the modifiers before typing any
 * digits, and enter the digits without modifiers.
 */
#define HEX_MOD_MASK (IBUS_CONTROL_MASK | IBUS_SHIFT_MASK)

static gboolean
check_hex (IBusSimpleEngine *simple,
           gint                 n_compose)
{
    gint i;
    GString *str;
    gulong n;
    gchar *nptr = NULL;
    gchar buf[7];

    simple->tentative_match = 0;
    simple->tentative_match_len = 0;

    str = g_string_new (NULL);

    i = 0;
    while (i < n_compose) {
        gunichar ch;

        ch = ibus_keyval_to_unicode (simple->compose_buffer[i]);

        if (ch == 0)
            return FALSE;

        if (!g_unichar_isxdigit (ch))
            return FALSE;

        buf[g_unichar_to_utf8 (ch, buf)] = '\0';

        g_string_append (str, buf);

        ++i;
    }

    n = strtoul (str->str, &nptr, 16);

    /* if strtoul fails it probably means non-latin digits were used;
     * we should in principle handle that, but we probably don't.
     */
    if (nptr - str->str < str->len) {
        g_string_free (str, TRUE);
        return FALSE;
    } else
        g_string_free (str, TRUE);

    if (g_unichar_validate (n)) {
        simple->tentative_match = n;
        simple->tentative_match_len = n_compose;
    }

    return TRUE;
}

static int
compare_seq_index (const void *key, const void *value)
{
    const guint *keysyms = key;
    const guint16 *seq = value;

    if (keysyms[0] < seq[0])
        return -1;
    else if (keysyms[0] > seq[0])
        return 1;
    return 0;
}

static int
compare_seq (const void *key, const void *value)
{
    int i = 0;
    const guint *keysyms = key;
    const guint16 *seq = value;

    while (keysyms[i]) {
        if (keysyms[i] < seq[i])
            return -1;
        else if (keysyms[i] > seq[i])
            return 1;

        i++;
    }

    return 0;
}


static gboolean
check_addon_table (IBusSimpleEngine *simple,
                   gint              n_compose)
{
    IBusComposeAddOnTableType table_type;
    const guint16 *data = NULL;
    gint max_seq_len = 0;
    gint n_seqs = 0;
    gint row_stride = 0;
    guint16 *seq;

    g_assert (IBUS_IS_SIMPLE_ENGINE (simple));

    table_type = IBUS_SIMPLE_ENGINE_GET_CLASS (simple)->compose_addon_table_type;

    if (table_type == IBUS_COMPOSE_TABLE_CEDILLA) {
        data = cedilla_compose_seqs;
        max_seq_len = 4;
        n_seqs = G_N_ELEMENTS (cedilla_compose_seqs) / (4 + 2);
    }
    else {
        return FALSE;
    }

    /* Will never match, if the sequence in the compose buffer is longer
     * than the sequences in the table.  Further, compare_seq (key, val)
     * will overrun val if key is longer than val. */
    if (n_compose > max_seq_len) {
        return FALSE;
    }

    row_stride = max_seq_len + 2;
    seq = bsearch (simple->compose_buffer,
                   data, n_seqs,
                   sizeof (guint16) *  row_stride,
                   compare_seq);

    if (seq) {
        guint16 *prev_seq;

        /* Back up to the first sequence that matches to make sure
         * we find the exact match if their is one.
         */
        while (seq > data) {
            prev_seq = seq - row_stride;
            if (compare_seq (simple->compose_buffer, prev_seq) != 0) {
                break;
            }
            seq = prev_seq;
        }

        /* complete sequence */
        if (n_compose == max_seq_len || seq[n_compose] == 0) {
            guint16 *next_seq;
            gunichar value =
                0x10000 * seq[max_seq_len] + seq[max_seq_len + 1];

            /* We found a tentative match. See if there are any longer
             * sequences containing this subsequence
             */
            next_seq = seq + row_stride;
            if (next_seq < data + row_stride * n_seqs) {
                if (compare_seq (simple->compose_buffer, next_seq) == 0) {
                    simple->tentative_match = value;
                    simple->tentative_match_len = n_compose;

                    ibus_simple_engine_update_preedit_text (simple);

                    return TRUE;
                }
            }

            ibus_simple_engine_commit_char (simple, value);
            g_debug ("U+%04X\n", value);
            simple->compose_buffer[0] = 0;
        }
        return TRUE;
    }
    return FALSE;
}

static gboolean
check_compact_table (IBusSimpleEngine          *simple,
                     const GtkComposeTableCompact *table,
                     gint                          n_compose)
{
    gint row_stride;
    guint16 *seq_index;
    guint16 *seq;
    gint i;

    /* Will never match, if the sequence in the compose buffer is longer
     * than the sequences in the table.  Further, compare_seq (key, val)
     * will overrun val if key is longer than val. */
    if (n_compose > table->max_seq_len)
        return FALSE;

    g_debug ("check_compact_table(n_compose=%d) [%04x, %04x, %04x, %04x]",
            n_compose,
            simple->compose_buffer[0],
            simple->compose_buffer[1],
            simple->compose_buffer[2],
            simple->compose_buffer[3]);

    seq_index = bsearch (simple->compose_buffer,
                         table->data,
                         table->n_index_size,
                         sizeof (guint16) *  table->n_index_stride,
                         compare_seq_index);

    if (seq_index == NULL) {
        g_debug ("compact: no\n");
        return FALSE;
    }

    if (n_compose == 1) {
        g_debug ("compact: yes\n");
        return TRUE;
    }

    g_debug ("compact: %04x ", *seq_index);
    seq = NULL;

    for (i = n_compose - 1; i < table->max_seq_len; i++) {
        row_stride = i + 1;

        if (seq_index[i + 1] - seq_index[i] > 0) {
            seq = bsearch (simple->compose_buffer + 1,
                           table->data + seq_index[i],
                           (seq_index[i + 1] - seq_index[i]) / row_stride,
                           sizeof (guint16) * row_stride,
                           compare_seq);
            g_debug ("seq = %p", seq);

            if (seq) {
                if (i == n_compose - 1)
                    break;
                else {
                    ibus_simple_engine_update_preedit_text (simple);
                    g_debug ("yes\n");
                    return TRUE;
                }
            }
        }
    }

    if (!seq) {
        g_debug ("no\n");
        return FALSE;
    }
    else {
        gunichar value;

        value = seq[row_stride - 1];
        ibus_simple_engine_commit_char (simple, value);
        simple->compose_buffer[0] = 0;

        g_debug ("U+%04X\n", value);
        return TRUE;
    }
}

/* Checks if a keysym is a dead key. Dead key keysym values are defined in
 * ../gdk/gdkkeysyms.h and the first is GDK_KEY_dead_grave. As X.Org is updated,
 * more dead keys are added and we need to update the upper limit.
 * Currently, the upper limit is GDK_KEY_dead_dasia+1. The +1 has to do with
 * a temporary issue in the X.Org header files.
 * In future versions it will be just the keysym (no +1).
 */
#define IS_DEAD_KEY(k) \
      ((k) >= IBUS_dead_grave && (k) <= (IBUS_dead_dasia+1))

/* This function receives a sequence of Unicode characters and tries to
 * normalize it (NFC). We check for the case the the resulting string
 * has length 1 (single character).
 * NFC normalisation normally rearranges diacritic marks, unless these
 * belong to the same Canonical Combining Class.
 * If they belong to the same canonical combining class, we produce all
 * permutations of the diacritic marks, then attempt to normalize.
 */
static gboolean
check_normalize_nfc (gunichar* combination_buffer, gint n_compose)
{
    gunichar combination_buffer_temp[MAX_COMPOSE_LEN];
    gchar *combination_utf8_temp = NULL;
    gchar *nfc_temp = NULL;
    gint n_combinations;
    gunichar temp_swap;
    gint i;

    n_combinations = 1;

    for (i = 1; i < n_compose; i++ )
        n_combinations *= i;

    /* Xorg reuses dead_tilde for the perispomeni diacritic mark.
     * We check if base character belongs to Greek Unicode block,
     * and if so, we replace tilde with perispomeni. */
    if (combination_buffer[0] >= 0x390 && combination_buffer[0] <= 0x3FF) {
        for (i = 1; i < n_compose; i++ )
            if (combination_buffer[i] == 0x303)
                combination_buffer[i] = 0x342;
    }

    memcpy (combination_buffer_temp, combination_buffer, MAX_COMPOSE_LEN * sizeof (gunichar) );

    for (i = 0; i < n_combinations; i++ ) {
        g_unicode_canonical_ordering (combination_buffer_temp, n_compose);
        combination_utf8_temp = g_ucs4_to_utf8 (combination_buffer_temp, -1, NULL, NULL, NULL);
        nfc_temp = g_utf8_normalize (combination_utf8_temp, -1, G_NORMALIZE_NFC);

        if (g_utf8_strlen (nfc_temp, -1) == 1) {
            memcpy (combination_buffer, combination_buffer_temp, MAX_COMPOSE_LEN * sizeof (gunichar) );

            g_free (combination_utf8_temp);
            g_free (nfc_temp);

            return TRUE;
        }

        g_free (combination_utf8_temp);
        g_free (nfc_temp);

        if (n_compose > 2) {
            temp_swap = combination_buffer_temp[i % (n_compose - 1) + 1];
            combination_buffer_temp[i % (n_compose - 1) + 1] = combination_buffer_temp[(i+1) % (n_compose - 1) + 1];
            combination_buffer_temp[(i+1) % (n_compose - 1) + 1] = temp_swap;
        }
        else
            break;
    }

    return FALSE;
}

static gboolean
check_algorithmically (IBusSimpleEngine *simple,
                       gint                n_compose)

{
    gint i;
    gunichar combination_buffer[MAX_COMPOSE_LEN];
    gchar *combination_utf8, *nfc;

    if (n_compose >= MAX_COMPOSE_LEN)
        return FALSE;

    for (i = 0; i < n_compose && IS_DEAD_KEY (simple->compose_buffer[i]); i++)
        ;
    if (i == n_compose)
        return TRUE;

    if (i > 0 && i == n_compose - 1) {
        combination_buffer[0] = ibus_keyval_to_unicode (simple->compose_buffer[i]);
        combination_buffer[n_compose] = 0;
        i--;
        while (i >= 0) {
        switch (simple->compose_buffer[i]) {
#define CASE(keysym, unicode) \
        case IBUS_dead_##keysym: combination_buffer[i+1] = unicode; break
        CASE (grave, 0x0300);
        CASE (acute, 0x0301);
        CASE (circumflex, 0x0302);
        CASE (tilde, 0x0303);    /* Also used with perispomeni, 0x342. */
        CASE (macron, 0x0304);
        CASE (breve, 0x0306);
        CASE (abovedot, 0x0307);
        CASE (diaeresis, 0x0308);
        CASE (hook, 0x0309);
        CASE (abovering, 0x030A);
        CASE (doubleacute, 0x030B);
        CASE (caron, 0x030C);
        CASE (abovecomma, 0x0313);         /* Equivalent to psili */
        CASE (abovereversedcomma, 0x0314); /* Equivalent to dasia */
        CASE (horn, 0x031B);    /* Legacy use for psili, 0x313 (or 0x343). */
        CASE (belowdot, 0x0323);
        CASE (cedilla, 0x0327);
        CASE (ogonek, 0x0328);    /* Legacy use for dasia, 0x314.*/
        CASE (iota, 0x0345);
        CASE (voiced_sound, 0x3099);    /* Per Markus Kuhn keysyms.txt file. */
        CASE (semivoiced_sound, 0x309A);    /* Per Markus Kuhn keysyms.txt file. */

        /* The following cases are to be removed once xkeyboard-config,
          * xorg are fully updated.
          */
            /* Workaround for typo in 1.4.x xserver-xorg */
        case 0xfe66: combination_buffer[i+1] = 0x314; break;
        /* CASE (dasia, 0x314); */
        /* CASE (perispomeni, 0x342); */
        /* CASE (psili, 0x343); */
#undef CASE
        default:
            combination_buffer[i+1] = ibus_keyval_to_unicode (simple->compose_buffer[i]);
        }
        i--;
    }

        /* If the buffer normalizes to a single character,
         * then modify the order of combination_buffer accordingly, if necessary,
         * and return TRUE.
         */
        if (check_normalize_nfc (combination_buffer, n_compose)) {
            gunichar value;
            combination_utf8 = g_ucs4_to_utf8 (combination_buffer, -1, NULL, NULL, NULL);
            nfc = g_utf8_normalize (combination_utf8, -1, G_NORMALIZE_NFC);

            value = g_utf8_get_char (nfc);
            ibus_simple_engine_commit_char (simple, value);
            simple->compose_buffer[0] = 0;

            g_free (combination_utf8);
            g_free (nfc);

            return TRUE;
        }
    }

    return FALSE;
}

static gboolean
no_sequence_matches (IBusSimpleEngine *simple,
                     gint                 n_compose,
                     guint                keyval,
                     guint                keycode,
                     guint                modifiers)
{
    gunichar ch;

    /* No compose sequences found, check first if we have a partial
     * match pending.
     */
    if (simple->tentative_match) {
        gint len = simple->tentative_match_len;
        int i;

        ibus_simple_engine_commit_char (simple,
                                            simple->tentative_match);
        simple->compose_buffer[0] = 0;

        for (i=0; i < n_compose - len - 1; i++) {
            ibus_simple_engine_process_key_event (
                    (IBusEngine *)simple,
                    simple->compose_buffer[len + i],
                    0, 0);
        }

        return ibus_simple_engine_process_key_event (
                (IBusEngine *)simple, keyval, keycode, modifiers);
    }
    else {
        simple->compose_buffer[0] = 0;
        if (n_compose > 1) {
            /* Invalid sequence */
            // FIXME beep_window (event->window);
            return TRUE;
        }

        ch = ibus_keyval_to_unicode (keyval);
        /* IBUS_CHANGE: RH#769133
         * Since we use ibus xkb engines as the disable state,
         * do not commit the characters locally without in_hex_sequence. */
        if (ch != 0 && !g_unichar_iscntrl (ch) &&
            simple->in_hex_sequence) {
            ibus_simple_engine_commit_char (simple, ch);
            return TRUE;
        }
        else
            return FALSE;
    }
}

gboolean
ibus_simple_engine_process_key_event (IBusEngine *engine,
                                      guint       keyval,
                                      guint       keycode,
                                      guint       modifiers)
{
    IBusSimpleEngine *simple = (IBusSimpleEngine *)engine;
    gint n_compose = 0;
    gboolean have_hex_mods;
    gboolean is_hex_start;
    gboolean is_hex_end;
    gboolean is_backspace;
    gboolean is_escape;
    guint hex_keyval;
    gint i;

    while (simple->compose_buffer[n_compose] != 0)
        n_compose++;

    if (n_compose >= MAX_COMPOSE_LEN)
        return TRUE;

    if (modifiers & IBUS_RELEASE_MASK) {
        if (simple->in_hex_sequence &&
            (keyval == IBUS_Control_L || keyval == IBUS_Control_R ||
             keyval == IBUS_Shift_L || keyval == IBUS_Shift_R)) {
            if (simple->tentative_match &&
                g_unichar_validate (simple->tentative_match)) {
                ibus_simple_engine_commit_char (simple,
                                                    simple->tentative_match);
            }
            else if (n_compose == 0) {
                simple->modifiers_dropped = TRUE;
            }
            else {
                /* invalid hex sequence */
                /* FIXME beep_window (event->window); */
                simple->tentative_match = 0;
                simple->in_hex_sequence = FALSE;
                simple->compose_buffer[0] = 0;

                ibus_simple_engine_update_preedit_text (simple);
            }

            return TRUE;
        }
        else
            return FALSE;
    }

    /* Ignore modifier key presses */
    for (i = 0; i < G_N_ELEMENTS (gtk_compose_ignore); i++)
        if (keyval == gtk_compose_ignore[i])
            return FALSE;

    if (simple->in_hex_sequence && simple->modifiers_dropped)
        have_hex_mods = TRUE;
    else
        have_hex_mods = (modifiers & (HEX_MOD_MASK)) == HEX_MOD_MASK;

    is_hex_start = keyval == IBUS_U;
    is_hex_end = (keyval == IBUS_space ||
                  keyval == IBUS_KP_Space ||
                  keyval == IBUS_Return ||
                  keyval == IBUS_ISO_Enter ||
                  keyval == IBUS_KP_Enter);
    is_backspace = keyval == IBUS_BackSpace;
    is_escape = keyval == IBUS_Escape;
    hex_keyval = keyval;

    /* If we are already in a non-hex sequence, or
     * this keystroke is not hex modifiers + hex digit, don't filter
     * key events with accelerator modifiers held down. We only treat
     * Control and Alt as accel modifiers here, since Super, Hyper and
     * Meta are often co-located with Mode_Switch, Multi_Key or
     * ISO_Level3_Switch.
     */
    if (!have_hex_mods ||
        (n_compose > 0 && !simple->in_hex_sequence) ||
        (n_compose == 0 && !simple->in_hex_sequence && !is_hex_start) ||
        (simple->in_hex_sequence && !hex_keyval &&
         !is_hex_start && !is_hex_end && !is_escape && !is_backspace)) {
        if (modifiers & (IBUS_MOD1_MASK | IBUS_CONTROL_MASK) ||
            (simple->in_hex_sequence && simple->modifiers_dropped &&
             (keyval == IBUS_Return ||
              keyval == IBUS_ISO_Enter ||
              keyval == IBUS_KP_Enter))) {
              return FALSE;
        }
    }

    /* Handle backspace */
    if (simple->in_hex_sequence && have_hex_mods && is_backspace) {
        if (n_compose > 0) {
        n_compose--;
            simple->compose_buffer[n_compose] = 0;
            check_hex (simple, n_compose);
    }
        else {
        simple->in_hex_sequence = FALSE;
    }

        ibus_simple_engine_update_preedit_text (simple);

        return TRUE;
    }

    /* Check for hex sequence restart */
    if (simple->in_hex_sequence && have_hex_mods && is_hex_start) {
        if (simple->tentative_match &&
            g_unichar_validate (simple->tentative_match)) {
            ibus_simple_engine_commit_char (simple, simple->tentative_match);
        }
        else {
            /* invalid hex sequence */
            if (n_compose > 0) {
                // FIXME beep_window (event->window);
                simple->tentative_match = 0;
                simple->in_hex_sequence = FALSE;
                simple->compose_buffer[0] = 0;
            }
        }
    }

    /* Check for hex sequence start */
    if (!simple->in_hex_sequence && have_hex_mods && is_hex_start) {
        simple->compose_buffer[0] = 0;
        simple->in_hex_sequence = TRUE;
        simple->modifiers_dropped = FALSE;
        simple->tentative_match = 0;

        g_debug ("Start HEX MODE");

        ibus_simple_engine_update_preedit_text (simple);

        return TRUE;
    }

    /* Then, check for compose sequences */
    if (simple->in_hex_sequence) {
        if (hex_keyval)
            simple->compose_buffer[n_compose++] = hex_keyval;
        else if (is_escape) {
            // FIXME
            ibus_simple_engine_reset (engine);

            return TRUE;
        }
        else if (!is_hex_end) {
            // FIXME
            /* non-hex character in hex sequence */
            // beep_window (event->window);
            return TRUE;
        }
    }
    else
        simple->compose_buffer[n_compose++] = keyval;

    simple->compose_buffer[n_compose] = 0;

    if (simple->in_hex_sequence) {
        /* If the modifiers are still held down, consider the sequence again */
        if (have_hex_mods) {
            /* space or return ends the sequence, and we eat the key */
            if (n_compose > 0 && is_hex_end) {
                if (simple->tentative_match &&
                    g_unichar_validate (simple->tentative_match)) {
                    ibus_simple_engine_commit_char (simple,
                            simple->tentative_match);
                    simple->compose_buffer[0] = 0;
                }
                else {
                    // FIXME
                    /* invalid hex sequence */
                    // beep_window (event->window);
                    simple->tentative_match = 0;
                    simple->in_hex_sequence = FALSE;
                    simple->compose_buffer[0] = 0;
                }
            }
            else if (!check_hex (simple, n_compose))
                // FIXME
                // beep_window (event->window);
                ;
            ibus_simple_engine_update_preedit_text (simple);

            return TRUE;
        }
    }
    else {
        // TODO CONT
        if (check_addon_table (simple, n_compose)) {
            return TRUE;
        }
        if (check_compact_table (simple, &gtk_compose_table_compact, n_compose))
            return TRUE;

        if (check_algorithmically (simple, n_compose))
            return TRUE;
    }

    /* The current compose_buffer doesn't match anything */
    return no_sequence_matches (simple, n_compose, keyval, keycode, modifiers);
}
