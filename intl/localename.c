/* Determine name of the currently selected locale.
   Copyright (C) 1995-1999, 2000-2007 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify it
   under the terms of the GNU Library General Public License as published
   by the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
   USA.  */

/* Written by Ulrich Drepper <drepper@gnu.org>, 1995.  */
/* Win32 code written by Tor Lillqvist <tml@iki.fi>.  */
/* MacOS X code written by Bruno Haible <bruno@clisp.org>.  */

#include <config.h>

/* Specification.  */
#ifdef IN_LIBINTL
# include "gettextP.h"
#else
# include "localename.h"
#endif

#include <stdlib.h>
#include <locale.h>

#if HAVE_CFLOCALECOPYCURRENT || HAVE_CFPREFERENCESCOPYAPPVALUE
# include <string.h>
# include <CoreFoundation/CFString.h>
# if HAVE_CFLOCALECOPYCURRENT
#  include <CoreFoundation/CFLocale.h>
# elif HAVE_CFPREFERENCESCOPYAPPVALUE
#  include <CoreFoundation/CFPreferences.h>
# endif
#endif

#if defined _WIN32 || defined __WIN32__
# define WIN32_NATIVE
#endif

#ifdef WIN32_NATIVE
# define WIN32_LEAN_AND_MEAN
# include <windows.h>
/* List of language codes, sorted by value:
   0x01 LANG_ARABIC
   0x02 LANG_BULGARIAN
   0x03 LANG_CATALAN
   0x04 LANG_CHINESE
   0x05 LANG_CZECH
   0x06 LANG_DANISH
   0x07 LANG_GERMAN
   0x08 LANG_GREEK
   0x09 LANG_ENGLISH
   0x0a LANG_SPANISH
   0x0b LANG_FINNISH
   0x0c LANG_FRENCH
   0x0d LANG_HEBREW
   0x0e LANG_HUNGARIAN
   0x0f LANG_ICELANDIC
   0x10 LANG_ITALIAN
   0x11 LANG_JAPANESE
   0x12 LANG_KOREAN
   0x13 LANG_DUTCH
   0x14 LANG_NORWEGIAN
   0x15 LANG_POLISH
   0x16 LANG_PORTUGUESE
   0x17 LANG_RHAETO_ROMANCE
   0x18 LANG_ROMANIAN
   0x19 LANG_RUSSIAN
   0x1a LANG_CROATIAN == LANG_SERBIAN
   0x1b LANG_SLOVAK
   0x1c LANG_ALBANIAN
   0x1d LANG_SWEDISH
   0x1e LANG_THAI
   0x1f LANG_TURKISH
   0x20 LANG_URDU
   0x21 LANG_INDONESIAN
   0x22 LANG_UKRAINIAN
   0x23 LANG_BELARUSIAN
   0x24 LANG_SLOVENIAN
   0x25 LANG_ESTONIAN
   0x26 LANG_LATVIAN
   0x27 LANG_LITHUANIAN
   0x28 LANG_TAJIK
   0x29 LANG_FARSI
   0x2a LANG_VIETNAMESE
   0x2b LANG_ARMENIAN
   0x2c LANG_AZERI
   0x2d LANG_BASQUE
   0x2e LANG_SORBIAN
   0x2f LANG_MACEDONIAN
   0x30 LANG_SUTU
   0x31 LANG_TSONGA
   0x32 LANG_TSWANA
   0x33 LANG_VENDA
   0x34 LANG_XHOSA
   0x35 LANG_ZULU
   0x36 LANG_AFRIKAANS
   0x37 LANG_GEORGIAN
   0x38 LANG_FAEROESE
   0x39 LANG_HINDI
   0x3a LANG_MALTESE
   0x3b LANG_SAAMI
   0x3c LANG_GAELIC
   0x3d LANG_YIDDISH
   0x3e LANG_MALAY
   0x3f LANG_KAZAK
   0x40 LANG_KYRGYZ
   0x41 LANG_SWAHILI
   0x42 LANG_TURKMEN
   0x43 LANG_UZBEK
   0x44 LANG_TATAR
   0x45 LANG_BENGALI
   0x46 LANG_PUNJABI
   0x47 LANG_GUJARATI
   0x48 LANG_ORIYA
   0x49 LANG_TAMIL
   0x4a LANG_TELUGU
   0x4b LANG_KANNADA
   0x4c LANG_MALAYALAM
   0x4d LANG_ASSAMESE
   0x4e LANG_MARATHI
   0x4f LANG_SANSKRIT
   0x50 LANG_MONGOLIAN
   0x51 LANG_TIBETAN
   0x52 LANG_WELSH
   0x53 LANG_CAMBODIAN
   0x54 LANG_LAO
   0x55 LANG_BURMESE
   0x56 LANG_GALICIAN
   0x57 LANG_KONKANI
   0x58 LANG_MANIPURI
   0x59 LANG_SINDHI
   0x5a LANG_SYRIAC
   0x5b LANG_SINHALESE
   0x5c LANG_CHEROKEE
   0x5d LANG_INUKTITUT
   0x5e LANG_AMHARIC
   0x5f LANG_TAMAZIGHT
   0x60 LANG_KASHMIRI
   0x61 LANG_NEPALI
   0x62 LANG_FRISIAN
   0x63 LANG_PASHTO
   0x64 LANG_TAGALOG
   0x65 LANG_DIVEHI
   0x66 LANG_EDO
   0x67 LANG_FULFULDE
   0x68 LANG_HAUSA
   0x69 LANG_IBIBIO
   0x6a LANG_YORUBA
   0x70 LANG_IGBO
   0x71 LANG_KANURI
   0x72 LANG_OROMO
   0x73 LANG_TIGRINYA
   0x74 LANG_GUARANI
   0x75 LANG_HAWAIIAN
   0x76 LANG_LATIN
   0x77 LANG_SOMALI
   0x78 LANG_YI
   0x79 LANG_PAPIAMENTU
*/
/* Mingw headers don't have latest language and sublanguage codes.  */
# ifndef LANG_AFRIKAANS
# define LANG_AFRIKAANS 0x36
# endif
# ifndef LANG_ALBANIAN
# define LANG_ALBANIAN 0x1c
# endif
# ifndef LANG_AMHARIC
# define LANG_AMHARIC 0x5e
# endif
# ifndef LANG_ARABIC
# define LANG_ARABIC 0x01
# endif
# ifndef LANG_ARMENIAN
# define LANG_ARMENIAN 0x2b
# endif
# ifndef LANG_ASSAMESE
# define LANG_ASSAMESE 0x4d
# endif
# ifndef LANG_AZERI
# define LANG_AZERI 0x2c
# endif
# ifndef LANG_BASQUE
# define LANG_BASQUE 0x2d
# endif
# ifndef LANG_BELARUSIAN
# define LANG_BELARUSIAN 0x23
# endif
# ifndef LANG_BENGALI
# define LANG_BENGALI 0x45
# endif
# ifndef LANG_BURMESE
# define LANG_BURMESE 0x55
# endif
# ifndef LANG_CAMBODIAN
# define LANG_CAMBODIAN 0x53
# endif
# ifndef LANG_CATALAN
# define LANG_CATALAN 0x03
# endif
# ifndef LANG_CHEROKEE
# define LANG_CHEROKEE 0x5c
# endif
# ifndef LANG_DIVEHI
# define LANG_DIVEHI 0x65
# endif
# ifndef LANG_EDO
# define LANG_EDO 0x66
# endif
# ifndef LANG_ESTONIAN
# define LANG_ESTONIAN 0x25
# endif
# ifndef LANG_FAEROESE
# define LANG_FAEROESE 0x38
# endif
# ifndef LANG_FARSI
# define LANG_FARSI 0x29
# endif
# ifndef LANG_FRISIAN
# define LANG_FRISIAN 0x62
# endif
# ifndef LANG_FULFULDE
# define LANG_FULFULDE 0x67
# endif
# ifndef LANG_GAELIC
# define LANG_GAELIC 0x3c
# endif
# ifndef LANG_GALICIAN
# define LANG_GALICIAN 0x56
# endif
# ifndef LANG_GEORGIAN
# define LANG_GEORGIAN 0x37
# endif
# ifndef LANG_GUARANI
# define LANG_GUARANI 0x74
# endif
# ifndef LANG_GUJARATI
# define LANG_GUJARATI 0x47
# endif
# ifndef LANG_HAUSA
# define LANG_HAUSA 0x68
# endif
# ifndef LANG_HAWAIIAN
# define LANG_HAWAIIAN 0x75
# endif
# ifndef LANG_HEBREW
# define LANG_HEBREW 0x0d
# endif
# ifndef LANG_HINDI
# define LANG_HINDI 0x39
# endif
# ifndef LANG_IBIBIO
# define LANG_IBIBIO 0x69
# endif
# ifndef LANG_IGBO
# define LANG_IGBO 0x70
# endif
# ifndef LANG_INDONESIAN
# define LANG_INDONESIAN 0x21
# endif
# ifndef LANG_INUKTITUT
# define LANG_INUKTITUT 0x5d
# endif
# ifndef LANG_KANNADA
# define LANG_KANNADA 0x4b
# endif
# ifndef LANG_KANURI
# define LANG_KANURI 0x71
# endif
# ifndef LANG_KASHMIRI
# define LANG_KASHMIRI 0x60
# endif
# ifndef LANG_KAZAK
# define LANG_KAZAK 0x3f
# endif
# ifndef LANG_KONKANI
# define LANG_KONKANI 0x57
# endif
# ifndef LANG_KYRGYZ
# define LANG_KYRGYZ 0x40
# endif
# ifndef LANG_LAO
# define LANG_LAO 0x54
# endif
# ifndef LANG_LATIN
# define LANG_LATIN 0x76
# endif
# ifndef LANG_LATVIAN
# define LANG_LATVIAN 0x26
# endif
# ifndef LANG_LITHUANIAN
# define LANG_LITHUANIAN 0x27
# endif
# ifndef LANG_MACEDONIAN
# define LANG_MACEDONIAN 0x2f
# endif
# ifndef LANG_MALAY
# define LANG_MALAY 0x3e
# endif
# ifndef LANG_MALAYALAM
# define LANG_MALAYALAM 0x4c
# endif
# ifndef LANG_MALTESE
# define LANG_MALTESE 0x3a
# endif
# ifndef LANG_MANIPURI
# define LANG_MANIPURI 0x58
# endif
# ifndef LANG_MARATHI
# define LANG_MARATHI 0x4e
# endif
# ifndef LANG_MONGOLIAN
# define LANG_MONGOLIAN 0x50
# endif
# ifndef LANG_NEPALI
# define LANG_NEPALI 0x61
# endif
# ifndef LANG_ORIYA
# define LANG_ORIYA 0x48
# endif
# ifndef LANG_OROMO
# define LANG_OROMO 0x72
# endif
# ifndef LANG_PAPIAMENTU
# define LANG_PAPIAMENTU 0x79
# endif
# ifndef LANG_PASHTO
# define LANG_PASHTO 0x63
# endif
# ifndef LANG_PUNJABI
# define LANG_PUNJABI 0x46
# endif
# ifndef LANG_RHAETO_ROMANCE
# define LANG_RHAETO_ROMANCE 0x17
# endif
# ifndef LANG_SAAMI
# define LANG_SAAMI 0x3b
# endif
# ifndef LANG_SANSKRIT
# define LANG_SANSKRIT 0x4f
# endif
# ifndef LANG_SERBIAN
# define LANG_SERBIAN 0x1a
# endif
# ifndef LANG_SINDHI
# define LANG_SINDHI 0x59
# endif
# ifndef LANG_SINHALESE
# define LANG_SINHALESE 0x5b
# endif
# ifndef LANG_SLOVAK
# define LANG_SLOVAK 0x1b
# endif
# ifndef LANG_SOMALI
# define LANG_SOMALI 0x77
# endif
# ifndef LANG_SORBIAN
# define LANG_SORBIAN 0x2e
# endif
# ifndef LANG_SUTU
# define LANG_SUTU 0x30
# endif
# ifndef LANG_SWAHILI
# define LANG_SWAHILI 0x41
# endif
# ifndef LANG_SYRIAC
# define LANG_SYRIAC 0x5a
# endif
# ifndef LANG_TAGALOG
# define LANG_TAGALOG 0x64
# endif
# ifndef LANG_TAJIK
# define LANG_TAJIK 0x28
# endif
# ifndef LANG_TAMAZIGHT
# define LANG_TAMAZIGHT 0x5f
# endif
# ifndef LANG_TAMIL
# define LANG_TAMIL 0x49
# endif
# ifndef LANG_TATAR
# define LANG_TATAR 0x44
# endif
# ifndef LANG_TELUGU
# define LANG_TELUGU 0x4a
# endif
# ifndef LANG_THAI
# define LANG_THAI 0x1e
# endif
# ifndef LANG_TIBETAN
# define LANG_TIBETAN 0x51
# endif
# ifndef LANG_TIGRINYA
# define LANG_TIGRINYA 0x73
# endif
# ifndef LANG_TSONGA
# define LANG_TSONGA 0x31
# endif
# ifndef LANG_TSWANA
# define LANG_TSWANA 0x32
# endif
# ifndef LANG_TURKMEN
# define LANG_TURKMEN 0x42
# endif
# ifndef LANG_UKRAINIAN
# define LANG_UKRAINIAN 0x22
# endif
# ifndef LANG_URDU
# define LANG_URDU 0x20
# endif
# ifndef LANG_UZBEK
# define LANG_UZBEK 0x43
# endif
# ifndef LANG_VENDA
# define LANG_VENDA 0x33
# endif
# ifndef LANG_VIETNAMESE
# define LANG_VIETNAMESE 0x2a
# endif
# ifndef LANG_WELSH
# define LANG_WELSH 0x52
# endif
# ifndef LANG_XHOSA
# define LANG_XHOSA 0x34
# endif
# ifndef LANG_YI
# define LANG_YI 0x78
# endif
# ifndef LANG_YIDDISH
# define LANG_YIDDISH 0x3d
# endif
# ifndef LANG_YORUBA
# define LANG_YORUBA 0x6a
# endif
# ifndef LANG_ZULU
# define LANG_ZULU 0x35
# endif
# ifndef SUBLANG_ARABIC_SAUDI_ARABIA
# define SUBLANG_ARABIC_SAUDI_ARABIA 0x01
# endif
# ifndef SUBLANG_ARABIC_IRAQ
# define SUBLANG_ARABIC_IRAQ 0x02
# endif
# ifndef SUBLANG_ARABIC_EGYPT
# define SUBLANG_ARABIC_EGYPT 0x03
# endif
# ifndef SUBLANG_ARABIC_LIBYA
# define SUBLANG_ARABIC_LIBYA 0x04
# endif
# ifndef SUBLANG_ARABIC_ALGERIA
# define SUBLANG_ARABIC_ALGERIA 0x05
# endif
# ifndef SUBLANG_ARABIC_MOROCCO
# define SUBLANG_ARABIC_MOROCCO 0x06
# endif
# ifndef SUBLANG_ARABIC_TUNISIA
# define SUBLANG_ARABIC_TUNISIA 0x07
# endif
# ifndef SUBLANG_ARABIC_OMAN
# define SUBLANG_ARABIC_OMAN 0x08
# endif
# ifndef SUBLANG_ARABIC_YEMEN
# define SUBLANG_ARABIC_YEMEN 0x09
# endif
# ifndef SUBLANG_ARABIC_SYRIA
# define SUBLANG_ARABIC_SYRIA 0x0a
# endif
# ifndef SUBLANG_ARABIC_JORDAN
# define SUBLANG_ARABIC_JORDAN 0x0b
# endif
# ifndef SUBLANG_ARABIC_LEBANON
# define SUBLANG_ARABIC_LEBANON 0x0c
# endif
# ifndef SUBLANG_ARABIC_KUWAIT
# define SUBLANG_ARABIC_KUWAIT 0x0d
# endif
# ifndef SUBLANG_ARABIC_UAE
# define SUBLANG_ARABIC_UAE 0x0e
# endif
# ifndef SUBLANG_ARABIC_BAHRAIN
# define SUBLANG_ARABIC_BAHRAIN 0x0f
# endif
# ifndef SUBLANG_ARABIC_QATAR
# define SUBLANG_ARABIC_QATAR 0x10
# endif
# ifndef SUBLANG_AZERI_LATIN
# define SUBLANG_AZERI_LATIN 0x01
# endif
# ifndef SUBLANG_AZERI_CYRILLIC
# define SUBLANG_AZERI_CYRILLIC 0x02
# endif
# ifndef SUBLANG_BENGALI_INDIA
# define SUBLANG_BENGALI_INDIA 0x01
# endif
# ifndef SUBLANG_BENGALI_BANGLADESH
# define SUBLANG_BENGALI_BANGLADESH 0x02
# endif
# ifndef SUBLANG_CHINESE_MACAU
# define SUBLANG_CHINESE_MACAU 0x05
# endif
# ifndef SUBLANG_ENGLISH_SOUTH_AFRICA
# define SUBLANG_ENGLISH_SOUTH_AFRICA 0x07
# endif
# ifndef SUBLANG_ENGLISH_JAMAICA
# define SUBLANG_ENGLISH_JAMAICA 0x08
# endif
# ifndef SUBLANG_ENGLISH_CARIBBEAN
# define SUBLANG_ENGLISH_CARIBBEAN 0x09
# endif
# ifndef SUBLANG_ENGLISH_BELIZE
# define SUBLANG_ENGLISH_BELIZE 0x0a
# endif
# ifndef SUBLANG_ENGLISH_TRINIDAD
# define SUBLANG_ENGLISH_TRINIDAD 0x0b
# endif
# ifndef SUBLANG_ENGLISH_ZIMBABWE
# define SUBLANG_ENGLISH_ZIMBABWE 0x0c
# endif
# ifndef SUBLANG_ENGLISH_PHILIPPINES
# define SUBLANG_ENGLISH_PHILIPPINES 0x0d
# endif
# ifndef SUBLANG_ENGLISH_INDONESIA
# define SUBLANG_ENGLISH_INDONESIA 0x0e
# endif
# ifndef SUBLANG_ENGLISH_HONGKONG
# define SUBLANG_ENGLISH_HONGKONG 0x0f
# endif
# ifndef SUBLANG_ENGLISH_INDIA
# define SUBLANG_ENGLISH_INDIA 0x10
# endif
# ifndef SUBLANG_ENGLISH_MALAYSIA
# define SUBLANG_ENGLISH_MALAYSIA 0x11
# endif
# ifndef SUBLANG_ENGLISH_SINGAPORE
# define SUBLANG_ENGLISH_SINGAPORE 0x12
# endif
# ifndef SUBLANG_FRENCH_LUXEMBOURG
# define SUBLANG_FRENCH_LUXEMBOURG 0x05
# endif
# ifndef SUBLANG_FRENCH_MONACO
# define SUBLANG_FRENCH_MONACO 0x06
# endif
# ifndef SUBLANG_FRENCH_WESTINDIES
# define SUBLANG_FRENCH_WESTINDIES 0x07
# endif
# ifndef SUBLANG_FRENCH_REUNION
# define SUBLANG_FRENCH_REUNION 0x08
# endif
# ifndef SUBLANG_FRENCH_CONGO
# define SUBLANG_FRENCH_CONGO 0x09
# endif
# ifndef SUBLANG_FRENCH_SENEGAL
# define SUBLANG_FRENCH_SENEGAL 0x0a
# endif
# ifndef SUBLANG_FRENCH_CAMEROON
# define SUBLANG_FRENCH_CAMEROON 0x0b
# endif
# ifndef SUBLANG_FRENCH_COTEDIVOIRE
# define SUBLANG_FRENCH_COTEDIVOIRE 0x0c
# endif
# ifndef SUBLANG_FRENCH_MALI
# define SUBLANG_FRENCH_MALI 0x0d
# endif
# ifndef SUBLANG_FRENCH_MOROCCO
# define SUBLANG_FRENCH_MOROCCO 0x0e
# endif
# ifndef SUBLANG_FRENCH_HAITI
# define SUBLANG_FRENCH_HAITI 0x0f
# endif
# ifndef SUBLANG_GERMAN_LUXEMBOURG
# define SUBLANG_GERMAN_LUXEMBOURG 0x04
# endif
# ifndef SUBLANG_GERMAN_LIECHTENSTEIN
# define SUBLANG_GERMAN_LIECHTENSTEIN 0x05
# endif
# ifndef SUBLANG_KASHMIRI_INDIA
# define SUBLANG_KASHMIRI_INDIA 0x02
# endif
# ifndef SUBLANG_MALAY_MALAYSIA
# define SUBLANG_MALAY_MALAYSIA 0x01
# endif
# ifndef SUBLANG_MALAY_BRUNEI_DARUSSALAM
# define SUBLANG_MALAY_BRUNEI_DARUSSALAM 0x02
# endif
# ifndef SUBLANG_NEPALI_INDIA
# define SUBLANG_NEPALI_INDIA 0x02
# endif
# ifndef SUBLANG_PUNJABI_INDIA
# define SUBLANG_PUNJABI_INDIA 0x01
# endif
# ifndef SUBLANG_PUNJABI_PAKISTAN
# define SUBLANG_PUNJABI_PAKISTAN 0x02
# endif
# ifndef SUBLANG_ROMANIAN_ROMANIA
# define SUBLANG_ROMANIAN_ROMANIA 0x01
# endif
# ifndef SUBLANG_ROMANIAN_MOLDOVA
# define SUBLANG_ROMANIAN_MOLDOVA 0x02
# endif
# ifndef SUBLANG_SERBIAN_LATIN
# define SUBLANG_SERBIAN_LATIN 0x02
# endif
# ifndef SUBLANG_SERBIAN_CYRILLIC
# define SUBLANG_SERBIAN_CYRILLIC 0x03
# endif
# ifndef SUBLANG_SINDHI_PAKISTAN
# define SUBLANG_SINDHI_PAKISTAN 0x01
# endif
# ifndef SUBLANG_SINDHI_AFGHANISTAN
# define SUBLANG_SINDHI_AFGHANISTAN 0x02
# endif
# ifndef SUBLANG_SPANISH_GUATEMALA
# define SUBLANG_SPANISH_GUATEMALA 0x04
# endif
# ifndef SUBLANG_SPANISH_COSTA_RICA
# define SUBLANG_SPANISH_COSTA_RICA 0x05
# endif
# ifndef SUBLANG_SPANISH_PANAMA
# define SUBLANG_SPANISH_PANAMA 0x06
# endif
# ifndef SUBLANG_SPANISH_DOMINICAN_REPUBLIC
# define SUBLANG_SPANISH_DOMINICAN_REPUBLIC 0x07
# endif
# ifndef SUBLANG_SPANISH_VENEZUELA
# define SUBLANG_SPANISH_VENEZUELA 0x08
# endif
# ifndef SUBLANG_SPANISH_COLOMBIA
# define SUBLANG_SPANISH_COLOMBIA 0x09
# endif
# ifndef SUBLANG_SPANISH_PERU
# define SUBLANG_SPANISH_PERU 0x0a
# endif
# ifndef SUBLANG_SPANISH_ARGENTINA
# define SUBLANG_SPANISH_ARGENTINA 0x0b
# endif
# ifndef SUBLANG_SPANISH_ECUADOR
# define SUBLANG_SPANISH_ECUADOR 0x0c
# endif
# ifndef SUBLANG_SPANISH_CHILE
# define SUBLANG_SPANISH_CHILE 0x0d
# endif
# ifndef SUBLANG_SPANISH_URUGUAY
# define SUBLANG_SPANISH_URUGUAY 0x0e
# endif
# ifndef SUBLANG_SPANISH_PARAGUAY
# define SUBLANG_SPANISH_PARAGUAY 0x0f
# endif
# ifndef SUBLANG_SPANISH_BOLIVIA
# define SUBLANG_SPANISH_BOLIVIA 0x10
# endif
# ifndef SUBLANG_SPANISH_EL_SALVADOR
# define SUBLANG_SPANISH_EL_SALVADOR 0x11
# endif
# ifndef SUBLANG_SPANISH_HONDURAS
# define SUBLANG_SPANISH_HONDURAS 0x12
# endif
# ifndef SUBLANG_SPANISH_NICARAGUA
# define SUBLANG_SPANISH_NICARAGUA 0x13
# endif
# ifndef SUBLANG_SPANISH_PUERTO_RICO
# define SUBLANG_SPANISH_PUERTO_RICO 0x14
# endif
# ifndef SUBLANG_SWEDISH_FINLAND
# define SUBLANG_SWEDISH_FINLAND 0x02
# endif
# ifndef SUBLANG_TAMAZIGHT_ARABIC
# define SUBLANG_TAMAZIGHT_ARABIC 0x01
# endif
# ifndef SUBLANG_TAMAZIGHT_ALGERIA_LATIN
# define SUBLANG_TAMAZIGHT_ALGERIA_LATIN 0x02
# endif
# ifndef SUBLANG_TIGRINYA_ETHIOPIA
# define SUBLANG_TIGRINYA_ETHIOPIA 0x01
# endif
# ifndef SUBLANG_TIGRINYA_ERITREA
# define SUBLANG_TIGRINYA_ERITREA 0x02
# endif
# ifndef SUBLANG_URDU_PAKISTAN
# define SUBLANG_URDU_PAKISTAN 0x01
# endif
# ifndef SUBLANG_URDU_INDIA
# define SUBLANG_URDU_INDIA 0x02
# endif
# ifndef SUBLANG_UZBEK_LATIN
# define SUBLANG_UZBEK_LATIN 0x01
# endif
# ifndef SUBLANG_UZBEK_CYRILLIC
# define SUBLANG_UZBEK_CYRILLIC 0x02
# endif
#endif

# if HAVE_CFLOCALECOPYCURRENT || HAVE_CFPREFERENCESCOPYAPPVALUE
/* MacOS X 10.2 or newer */

/* Canonicalize a MacOS X locale name to a Unix locale name.
   NAME is a sufficiently large buffer.
   On input, it contains the MacOS X locale name.
   On output, it contains the Unix locale name.  */
#  if !defined IN_LIBINTL
static
#  endif
void
gl_locale_name_canonicalize (char *name)
{
  /* This conversion is based on a posting by
     Deborah GoldSmith <goldsmit@apple.com> on 2005-03-08,
     http://lists.apple.com/archives/carbon-dev/2005/Mar/msg00293.html */

  /* Convert legacy (NeXTstep inherited) English names to Unix (ISO 639 and
     ISO 3166) names.  Prior to MacOS X 10.3, there is no API for doing this.
     Therefore we do it ourselves, using a table based on the results of the
     MacOS X 10.3.8 function
     CFLocaleCreateCanonicalLocaleIdentifierFromString().  */
  typedef struct { const char legacy[21+1]; const char unixy[5+1]; }
	  legacy_entry;
  static const legacy_entry legacy_table[] = {
    { "Afrikaans",             "af" },
    { "Albanian",              "sq" },
    { "Amharic",               "am" },
    { "Arabic",                "ar" },
    { "Armenian",              "hy" },
    { "Assamese",              "as" },
    { "Aymara",                "ay" },
    { "Azerbaijani",           "az" },
    { "Basque",                "eu" },
    { "Belarusian",            "be" },
    { "Belorussian",           "be" },
    { "Bengali",               "bn" },
    { "Brazilian Portugese",   "pt_BR" },
    { "Brazilian Portuguese",  "pt_BR" },
    { "Breton",                "br" },
    { "Bulgarian",             "bg" },
    { "Burmese",               "my" },
    { "Byelorussian",          "be" },
    { "Catalan",               "ca" },
    { "Chewa",                 "ny" },
    { "Chichewa",              "ny" },
    { "Chinese",               "zh" },
    { "Chinese, Simplified",   "zh_CN" },
    { "Chinese, Traditional",  "zh_TW" },
    { "Chinese, Tradtional",   "zh_TW" },
    { "Croatian",              "hr" },
    { "Czech",                 "cs" },
    { "Danish",                "da" },
    { "Dutch",                 "nl" },
    { "Dzongkha",              "dz" },
    { "English",               "en" },
    { "Esperanto",             "eo" },
    { "Estonian",              "et" },
    { "Faroese",               "fo" },
    { "Farsi",                 "fa" },
    { "Finnish",               "fi" },
    { "Flemish",               "nl_BE" },
    { "French",                "fr" },
    { "Galician",              "gl" },
    { "Gallegan",              "gl" },
    { "Georgian",              "ka" },
    { "German",                "de" },
    { "Greek",                 "el" },
    { "Greenlandic",           "kl" },
    { "Guarani",               "gn" },
    { "Gujarati",              "gu" },
    { "Hawaiian",              "haw" }, /* Yes, "haw", not "cpe".  */
    { "Hebrew",                "he" },
    { "Hindi",                 "hi" },
    { "Hungarian",             "hu" },
    { "Icelandic",             "is" },
    { "Indonesian",            "id" },
    { "Inuktitut",             "iu" },
    { "Irish",                 "ga" },
    { "Italian",               "it" },
    { "Japanese",              "ja" },
    { "Javanese",              "jv" },
    { "Kalaallisut",           "kl" },
    { "Kannada",               "kn" },
    { "Kashmiri",              "ks" },
    { "Kazakh",                "kk" },
    { "Khmer",                 "km" },
    { "Kinyarwanda",           "rw" },
    { "Kirghiz",               "ky" },
    { "Korean",                "ko" },
    { "Kurdish",               "ku" },
    { "Latin",                 "la" },
    { "Latvian",               "lv" },
    { "Lithuanian",            "lt" },
    { "Macedonian",            "mk" },
    { "Malagasy",              "mg" },
    { "Malay",                 "ms" },
    { "Malayalam",             "ml" },
    { "Maltese",               "mt" },
    { "Manx",                  "gv" },
    { "Marathi",               "mr" },
    { "Moldavian",             "mo" },
    { "Mongolian",             "mn" },
    { "Nepali",                "ne" },
    { "Norwegian",             "nb" }, /* Yes, "nb", not the obsolete "no".  */
    { "Nyanja",                "ny" },
    { "Nynorsk",               "nn" },
    { "Oriya",                 "or" },
    { "Oromo",                 "om" },
    { "Panjabi",               "pa" },
    { "Pashto",                "ps" },
    { "Persian",               "fa" },
    { "Polish",                "pl" },
    { "Portuguese",            "pt" },
    { "Portuguese, Brazilian", "pt_BR" },
    { "Punjabi",               "pa" },
    { "Pushto",                "ps" },
    { "Quechua",               "qu" },
    { "Romanian",              "ro" },
    { "Ruanda",                "rw" },
    { "Rundi",                 "rn" },
    { "Russian",               "ru" },
    { "Sami",                  "se_NO" }, /* Not just "se".  */
    { "Sanskrit",              "sa" },
    { "Scottish",              "gd" },
    { "Serbian",               "sr" },
    { "Simplified Chinese",    "zh_CN" },
    { "Sindhi",                "sd" },
    { "Sinhalese",             "si" },
    { "Slovak",                "sk" },
    { "Slovenian",             "sl" },
    { "Somali",                "so" },
    { "Spanish",               "es" },
    { "Sundanese",             "su" },
    { "Swahili",               "sw" },
    { "Swedish",               "sv" },
    { "Tagalog",               "tl" },
    { "Tajik",                 "tg" },
    { "Tajiki",                "tg" },
    { "Tamil",                 "ta" },
    { "Tatar",                 "tt" },
    { "Telugu",                "te" },
    { "Thai",                  "th" },
    { "Tibetan",               "bo" },
    { "Tigrinya",              "ti" },
    { "Tongan",                "to" },
    { "Traditional Chinese",   "zh_TW" },
    { "Turkish",               "tr" },
    { "Turkmen",               "tk" },
    { "Uighur",                "ug" },
    { "Ukrainian",             "uk" },
    { "Urdu",                  "ur" },
    { "Uzbek",                 "uz" },
    { "Vietnamese",            "vi" },
    { "Welsh",                 "cy" },
    { "Yiddish",               "yi" }
  };

  /* Convert new-style locale names with language tags (ISO 639 and ISO 15924)
     to Unix (ISO 639 and ISO 3166) names.  */
  typedef struct { const char langtag[7+1]; const char unixy[12+1]; }
	  langtag_entry;
  static const langtag_entry langtag_table[] = {
    /* MacOS X has "az-Arab", "az-Cyrl", "az-Latn".
       The default script for az on Unix is Latin.  */
    { "az-Latn", "az" },
    /* MacOS X has "ga-dots".  Does not yet exist on Unix.  */
    { "ga-dots", "ga" },
    /* MacOS X has "kk-Cyrl".  Does not yet exist on Unix.  */
    /* MacOS X has "mn-Cyrl", "mn-Mong".
       The default script for mn on Unix is Cyrillic.  */
    { "mn-Cyrl", "mn" },
    /* MacOS X has "ms-Arab", "ms-Latn".
       The default script for ms on Unix is Latin.  */
    { "ms-Latn", "ms" },
    /* MacOS X has "tg-Cyrl".
       The default script for tg on Unix is Cyrillic.  */
    { "tg-Cyrl", "tg" },
    /* MacOS X has "tk-Cyrl".  Does not yet exist on Unix.  */
    /* MacOS X has "tt-Cyrl".
       The default script for tt on Unix is Cyrillic.  */
    { "tt-Cyrl", "tt" },
    /* MacOS X has "zh-Hans", "zh-Hant".
       Country codes are used to distinguish these on Unix.  */
    { "zh-Hans", "zh_CN" },
    { "zh-Hant", "zh_TW" }
  };

  /* Convert script names (ISO 15924) to Unix conventions.
     See http://www.unicode.org/iso15924/iso15924-codes.html  */
  typedef struct { const char script[4+1]; const char unixy[9+1]; }
	  script_entry;
  static const script_entry script_table[] = {
    { "Arab", "arabic" },
    { "Cyrl", "cyrillic" },
    { "Mong", "mongolian" }
  };

  /* Step 1: Convert using legacy_table.  */
  if (name[0] >= 'A' && name[0] <= 'Z')
    {
      unsigned int i1, i2;
      i1 = 0;
      i2 = sizeof (legacy_table) / sizeof (legacy_entry);
      while (i2 - i1 > 1)
	{
	  /* At this point we know that if name occurs in legacy_table,
	     its index must be >= i1 and < i2.  */
	  unsigned int i = (i1 + i2) >> 1;
	  const legacy_entry *p = &legacy_table[i];
	  if (strcmp (name, p->legacy) < 0)
	    i2 = i;
	  else
	    i1 = i;
	}
      if (strcmp (name, legacy_table[i1].legacy) == 0)
	{
	  strcpy (name, legacy_table[i1].unixy);
	  return;
	}
    }

  /* Step 2: Convert using langtag_table and script_table.  */
  if (strlen (name) == 7 && name[2] == '-')
    {
      unsigned int i1, i2;
      i1 = 0;
      i2 = sizeof (langtag_table) / sizeof (langtag_entry);
      while (i2 - i1 > 1)
	{
	  /* At this point we know that if name occurs in langtag_table,
	     its index must be >= i1 and < i2.  */
	  unsigned int i = (i1 + i2) >> 1;
	  const langtag_entry *p = &langtag_table[i];
	  if (strcmp (name, p->langtag) < 0)
	    i2 = i;
	  else
	    i1 = i;
	}
      if (strcmp (name, langtag_table[i1].langtag) == 0)
	{
	  strcpy (name, langtag_table[i1].unixy);
	  return;
	}

      i1 = 0;
      i2 = sizeof (script_table) / sizeof (script_entry);
      while (i2 - i1 > 1)
	{
	  /* At this point we know that if (name + 3) occurs in script_table,
	     its index must be >= i1 and < i2.  */
	  unsigned int i = (i1 + i2) >> 1;
	  const script_entry *p = &script_table[i];
	  if (strcmp (name + 3, p->script) < 0)
	    i2 = i;
	  else
	    i1 = i;
	}
      if (strcmp (name + 3, script_table[i1].script) == 0)
	{
	  name[2] = '@';
	  strcpy (name + 3, script_table[i1].unixy);
	  return;
	}
    }

  /* Step 3: Convert new-style dash to Unix underscore. */
  {
    char *p;
    for (p = name; *p != '\0'; p++)
      if (*p == '-')
	*p = '_';
  }
}

#endif

/* XPG3 defines the result of 'setlocale (category, NULL)' as:
   "Directs 'setlocale()' to query 'category' and return the current
    setting of 'local'."
   However it does not specify the exact format.  Neither do SUSV2 and
   ISO C 99.  So we can use this feature only on selected systems (e.g.
   those using GNU C Library).  */
#if defined _LIBC || (defined __GLIBC__ && __GLIBC__ >= 2)
# define HAVE_LOCALE_NULL
#endif

/* Determine the current locale's name, and canonicalize it into XPG syntax
     language[_territory][.codeset][@modifier]
   The codeset part in the result is not reliable; the locale_charset()
   should be used for codeset information instead.
   The result must not be freed; it is statically allocated.  */

const char *
gl_locale_name_posix (int category, const char *categoryname)
{
  /* Use the POSIX methods of looking to 'LC_ALL', 'LC_xxx', and 'LANG'.
     On some systems this can be done by the 'setlocale' function itself.  */
#if defined HAVE_SETLOCALE && defined HAVE_LC_MESSAGES && defined HAVE_LOCALE_NULL
  return setlocale (category, NULL);
#else
  const char *retval;

  /* Setting of LC_ALL overrides all other.  */
  retval = getenv ("LC_ALL");
  if (retval != NULL && retval[0] != '\0')
    return retval;
  /* Next comes the name of the desired category.  */
  retval = getenv (categoryname);
  if (retval != NULL && retval[0] != '\0')
    return retval;
  /* Last possibility is the LANG environment variable.  */
  retval = getenv ("LANG");
  if (retval != NULL && retval[0] != '\0')
    return retval;

  return NULL;
#endif
}

const char *
gl_locale_name_default (void)
{
  /* POSIX:2001 says:
     "All implementations shall define a locale as the default locale, to be
      invoked when no environment variables are set, or set to the empty
      string.  This default locale can be the POSIX locale or any other
      implementation-defined locale.  Some implementations may provide
      facilities for local installation administrators to set the default
      locale, customizing it for each location.  POSIX:2001 does not require
      such a facility.  */

#if !(HAVE_CFLOCALECOPYCURRENT || HAVE_CFPREFERENCESCOPYAPPVALUE || defined(WIN32_NATIVE))

  /* The system does not have a way of setting the locale, other than the
     POSIX specified environment variables.  We use C as default locale.  */
  return "C";

#else

  /* Return an XPG style locale name language[_territory][@modifier].
     Don't even bother determining the codeset; it's not useful in this
     context, because message catalogs are not specific to a single
     codeset.  */

# if HAVE_CFLOCALECOPYCURRENT || HAVE_CFPREFERENCESCOPYAPPVALUE
  /* MacOS X 10.2 or newer */
  {
    /* Cache the locale name, since CoreFoundation calls are expensive.  */
    static const char *cached_localename;

    if (cached_localename == NULL)
      {
	char namebuf[256];
#  if HAVE_CFLOCALECOPYCURRENT /* MacOS X 10.3 or newer */
	CFLocaleRef locale = CFLocaleCopyCurrent ();
	CFStringRef name = CFLocaleGetIdentifier (locale);

	if (CFStringGetCString (name, namebuf, sizeof(namebuf),
				kCFStringEncodingASCII))
	  {
	    gl_locale_name_canonicalize (namebuf);
	    cached_localename = strdup (namebuf);
	  }
	CFRelease (locale);
#  elif HAVE_CFPREFERENCESCOPYAPPVALUE /* MacOS X 10.2 or newer */
	CFTypeRef value =
	  CFPreferencesCopyAppValue (CFSTR ("AppleLocale"),
				     kCFPreferencesCurrentApplication);
	if (value != NULL
	    && CFGetTypeID (value) == CFStringGetTypeID ()
	    && CFStringGetCString ((CFStringRef)value, namebuf, sizeof(namebuf),
				   kCFStringEncodingASCII))
	  {
	    gl_locale_name_canonicalize (namebuf);
	    cached_localename = strdup (namebuf);
	  }
#  endif
	if (cached_localename == NULL)
	  cached_localename = "C";
      }
    return cached_localename;
  }

# endif

# if defined(WIN32_NATIVE) /* WIN32, not Cygwin */
  {
    LCID lcid;
    LANGID langid;
    int primary, sub;

    /* Use native Win32 API locale ID.  */
    lcid = GetThreadLocale ();

    /* Strip off the sorting rules, keep only the language part.  */
    langid = LANGIDFROMLCID (lcid);

    /* Split into language and territory part.  */
    primary = PRIMARYLANGID (langid);
    sub = SUBLANGID (langid);

    /* Dispatch on language.
       See also http://www.unicode.org/unicode/onlinedat/languages.html .
       For details about languages, see http://www.ethnologue.com/ .  */
    switch (primary)
      {
      case LANG_AFRIKAANS: return "af_ZA";
      case LANG_ALBANIAN: return "sq_AL";
      case LANG_AMHARIC: return "am_ET";
      case LANG_ARABIC:
	switch (sub)
	  {
	  case SUBLANG_ARABIC_SAUDI_ARABIA: return "ar_SA";
	  case SUBLANG_ARABIC_IRAQ: return "ar_IQ";
	  case SUBLANG_ARABIC_EGYPT: return "ar_EG";
	  case SUBLANG_ARABIC_LIBYA: return "ar_LY";
	  case SUBLANG_ARABIC_ALGERIA: return "ar_DZ";
	  case SUBLANG_ARABIC_MOROCCO: return "ar_MA";
	  case SUBLANG_ARABIC_TUNISIA: return "ar_TN";
	  case SUBLANG_ARABIC_OMAN: return "ar_OM";
	  case SUBLANG_ARABIC_YEMEN: return "ar_YE";
	  case SUBLANG_ARABIC_SYRIA: return "ar_SY";
	  case SUBLANG_ARABIC_JORDAN: return "ar_JO";
	  case SUBLANG_ARABIC_LEBANON: return "ar_LB";
	  case SUBLANG_ARABIC_KUWAIT: return "ar_KW";
	  case SUBLANG_ARABIC_UAE: return "ar_AE";
	  case SUBLANG_ARABIC_BAHRAIN: return "ar_BH";
	  case SUBLANG_ARABIC_QATAR: return "ar_QA";
	  }
	return "ar";
      case LANG_ARMENIAN: return "hy_AM";
      case LANG_ASSAMESE: return "as_IN";
      case LANG_AZERI:
	switch (sub)
	  {
	  /* FIXME: Adjust this when Azerbaijani locales appear on Unix.  */
	  case SUBLANG_AZERI_LATIN: return "az_AZ@latin";
	  case SUBLANG_AZERI_CYRILLIC: return "az_AZ@cyrillic";
	  }
	return "az";
      case LANG_BASQUE:
	switch (sub)
	  {
	  case SUBLANG_DEFAULT: return "eu_ES";
	  }
	return "eu"; /* Ambiguous: could be "eu_ES" or "eu_FR".  */
      case LANG_BELARUSIAN: return "be_BY";
      case LANG_BENGALI:
	switch (sub)
	  {
	  case SUBLANG_BENGALI_INDIA: return "bn_IN";
	  case SUBLANG_BENGALI_BANGLADESH: return "bn_BD";
	  }
	return "bn";
      case LANG_BULGARIAN: return "bg_BG";
      case LANG_BURMESE: return "my_MM";
      case LANG_CAMBODIAN: return "km_KH";
      case LANG_CATALAN: return "ca_ES";
      case LANG_CHEROKEE: return "chr_US";
      case LANG_CHINESE:
	switch (sub)
	  {
	  case SUBLANG_CHINESE_TRADITIONAL: return "zh_TW";
	  case SUBLANG_CHINESE_SIMPLIFIED: return "zh_CN";
	  case SUBLANG_CHINESE_HONGKONG: return "zh_HK";
	  case SUBLANG_CHINESE_SINGAPORE: return "zh_SG";
	  case SUBLANG_CHINESE_MACAU: return "zh_MO";
	  }
	return "zh";
      case LANG_CROATIAN:       /* LANG_CROATIAN == LANG_SERBIAN
				 * What used to be called Serbo-Croatian
				 * should really now be two separate
				 * languages because of political reasons.
				 * (Says tml, who knows nothing about Serbian
				 * or Croatian.)
				 * (I can feel those flames coming already.)
				 */
	switch (sub)
	  {
	  case SUBLANG_DEFAULT: return "hr_HR";
	  case SUBLANG_SERBIAN_LATIN: return "sr_CS";
	  case SUBLANG_SERBIAN_CYRILLIC: return "sr_CS@cyrillic";
	  }
	return "hr";
      case LANG_CZECH: return "cs_CZ";
      case LANG_DANISH: return "da_DK";
      case LANG_DIVEHI: return "dv_MV";
      case LANG_DUTCH:
	switch (sub)
	  {
	  case SUBLANG_DUTCH: return "nl_NL";
	  case SUBLANG_DUTCH_BELGIAN: /* FLEMISH, VLAAMS */ return "nl_BE";
	  }
	return "nl";
      case LANG_EDO: return "bin_NG";
      case LANG_ENGLISH:
	switch (sub)
	  {
	  /* SUBLANG_ENGLISH_US == SUBLANG_DEFAULT. Heh. I thought
	   * English was the language spoken in England.
	   * Oh well.
	   */
	  case SUBLANG_ENGLISH_US: return "en_US";
	  case SUBLANG_ENGLISH_UK: return "en_GB";
	  case SUBLANG_ENGLISH_AUS: return "en_AU";
	  case SUBLANG_ENGLISH_CAN: return "en_CA";
	  case SUBLANG_ENGLISH_NZ: return "en_NZ";
	  case SUBLANG_ENGLISH_EIRE: return "en_IE";
	  case SUBLANG_ENGLISH_SOUTH_AFRICA: return "en_ZA";
	  case SUBLANG_ENGLISH_JAMAICA: return "en_JM";
	  case SUBLANG_ENGLISH_CARIBBEAN: return "en_GD"; /* Grenada? */
	  case SUBLANG_ENGLISH_BELIZE: return "en_BZ";
	  case SUBLANG_ENGLISH_TRINIDAD: return "en_TT";
	  case SUBLANG_ENGLISH_ZIMBABWE: return "en_ZW";
	  case SUBLANG_ENGLISH_PHILIPPINES: return "en_PH";
	  case SUBLANG_ENGLISH_INDONESIA: return "en_ID";
	  case SUBLANG_ENGLISH_HONGKONG: return "en_HK";
	  case SUBLANG_ENGLISH_INDIA: return "en_IN";
	  case SUBLANG_ENGLISH_MALAYSIA: return "en_MY";
	  case SUBLANG_ENGLISH_SINGAPORE: return "en_SG";
	  }
	return "en";
      case LANG_ESTONIAN: return "et_EE";
      case LANG_FAEROESE: return "fo_FO";
      case LANG_FARSI: return "fa_IR";
      case LANG_FINNISH: return "fi_FI";
      case LANG_FRENCH:
	switch (sub)
	  {
	  case SUBLANG_FRENCH: return "fr_FR";
	  case SUBLANG_FRENCH_BELGIAN: /* WALLOON */ return "fr_BE";
	  case SUBLANG_FRENCH_CANADIAN: return "fr_CA";
	  case SUBLANG_FRENCH_SWISS: return "fr_CH";
	  case SUBLANG_FRENCH_LUXEMBOURG: return "fr_LU";
	  case SUBLANG_FRENCH_MONACO: return "fr_MC";
	  case SUBLANG_FRENCH_WESTINDIES: return "fr"; /* Caribbean? */
	  case SUBLANG_FRENCH_REUNION: return "fr_RE";
	  case SUBLANG_FRENCH_CONGO: return "fr_CG";
	  case SUBLANG_FRENCH_SENEGAL: return "fr_SN";
	  case SUBLANG_FRENCH_CAMEROON: return "fr_CM";
	  case SUBLANG_FRENCH_COTEDIVOIRE: return "fr_CI";
	  case SUBLANG_FRENCH_MALI: return "fr_ML";
	  case SUBLANG_FRENCH_MOROCCO: return "fr_MA";
	  case SUBLANG_FRENCH_HAITI: return "fr_HT";
	  }
	return "fr";
      case LANG_FRISIAN: return "fy_NL";
      case LANG_FULFULDE:
	/* Spoken in Nigeria, Guinea, Senegal, Mali, Niger, Cameroon, Benin.  */
	return "ff_NG";
      case LANG_GAELIC:
	switch (sub)
	  {
	  case 0x01: /* SCOTTISH */ return "gd_GB";
	  case 0x02: /* IRISH */ return "ga_IE";
	  }
	return "C";
      case LANG_GALICIAN: return "gl_ES";
      case LANG_GEORGIAN: return "ka_GE";
      case LANG_GERMAN:
	switch (sub)
	  {
	  case SUBLANG_GERMAN: return "de_DE";
	  case SUBLANG_GERMAN_SWISS: return "de_CH";
	  case SUBLANG_GERMAN_AUSTRIAN: return "de_AT";
	  case SUBLANG_GERMAN_LUXEMBOURG: return "de_LU";
	  case SUBLANG_GERMAN_LIECHTENSTEIN: return "de_LI";
	  }
	return "de";
      case LANG_GREEK: return "el_GR";
      case LANG_GUARANI: return "gn_PY";
      case LANG_GUJARATI: return "gu_IN";
      case LANG_HAUSA: return "ha_NG";
      case LANG_HAWAIIAN:
	/* FIXME: Do they mean Hawaiian ("haw_US", 1000 speakers)
	   or Hawaii Creole English ("cpe_US", 600000 speakers)?  */
	return "cpe_US";
      case LANG_HEBREW: return "he_IL";
      case LANG_HINDI: return "hi_IN";
      case LANG_HUNGARIAN: return "hu_HU";
      case LANG_IBIBIO: return "nic_NG";
      case LANG_ICELANDIC: return "is_IS";
      case LANG_IGBO: return "ig_NG";
      case LANG_INDONESIAN: return "id_ID";
      case LANG_INUKTITUT: return "iu_CA";
      case LANG_ITALIAN:
	switch (sub)
	  {
	  case SUBLANG_ITALIAN: return "it_IT";
	  case SUBLANG_ITALIAN_SWISS: return "it_CH";
	  }
	return "it";
      case LANG_JAPANESE: return "ja_JP";
      case LANG_KANNADA: return "kn_IN";
      case LANG_KANURI: return "kr_NG";
      case LANG_KASHMIRI:
	switch (sub)
	  {
	  case SUBLANG_DEFAULT: return "ks_PK";
	  case SUBLANG_KASHMIRI_INDIA: return "ks_IN";
	  }
	return "ks";
      case LANG_KAZAK: return "kk_KZ";
      case LANG_KONKANI:
	/* FIXME: Adjust this when such locales appear on Unix.  */
	return "kok_IN";
      case LANG_KOREAN: return "ko_KR";
      case LANG_KYRGYZ: return "ky_KG";
      case LANG_LAO: return "lo_LA";
      case LANG_LATIN: return "la_VA";
      case LANG_LATVIAN: return "lv_LV";
      case LANG_LITHUANIAN: return "lt_LT";
      case LANG_MACEDONIAN: return "mk_MK";
      case LANG_MALAY:
	switch (sub)
	  {
	  case SUBLANG_MALAY_MALAYSIA: return "ms_MY";
	  case SUBLANG_MALAY_BRUNEI_DARUSSALAM: return "ms_BN";
	  }
	return "ms";
      case LANG_MALAYALAM: return "ml_IN";
      case LANG_MALTESE: return "mt_MT";
      case LANG_MANIPURI:
	/* FIXME: Adjust this when such locales appear on Unix.  */
	return "mni_IN";
      case LANG_MARATHI: return "mr_IN";
      case LANG_MONGOLIAN:
	switch (sub)
	  {
	  case SUBLANG_DEFAULT: return "mn_MN";
	  }
	return "mn"; /* Ambiguous: could be "mn_CN" or "mn_MN".  */
      case LANG_NEPALI:
	switch (sub)
	  {
	  case SUBLANG_DEFAULT: return "ne_NP";
	  case SUBLANG_NEPALI_INDIA: return "ne_IN";
	  }
	return "ne";
      case LANG_NORWEGIAN:
	switch (sub)
	  {
	  case SUBLANG_NORWEGIAN_BOKMAL: return "nb_NO";
	  case SUBLANG_NORWEGIAN_NYNORSK: return "nn_NO";
	  }
	return "no";
      case LANG_ORIYA: return "or_IN";
      case LANG_OROMO: return "om_ET";
      case LANG_PAPIAMENTU: return "pap_AN";
      case LANG_PASHTO:
	return "ps"; /* Ambiguous: could be "ps_PK" or "ps_AF".  */
      case LANG_POLISH: return "pl_PL";
      case LANG_PORTUGUESE:
	switch (sub)
	  {
	  case SUBLANG_PORTUGUESE: return "pt_PT";
	  /* Hmm. SUBLANG_PORTUGUESE_BRAZILIAN == SUBLANG_DEFAULT.
	     Same phenomenon as SUBLANG_ENGLISH_US == SUBLANG_DEFAULT. */
	  case SUBLANG_PORTUGUESE_BRAZILIAN: return "pt_BR";
	  }
	return "pt";
      case LANG_PUNJABI:
	switch (sub)
	  {
	  case SUBLANG_PUNJABI_INDIA: return "pa_IN"; /* Gurmukhi script */
	  case SUBLANG_PUNJABI_PAKISTAN: return "pa_PK"; /* Arabic script */
	  }
	return "pa";
      case LANG_RHAETO_ROMANCE: return "rm_CH";
      case LANG_ROMANIAN:
	switch (sub)
	  {
	  case SUBLANG_ROMANIAN_ROMANIA: return "ro_RO";
	  case SUBLANG_ROMANIAN_MOLDOVA: return "ro_MD";
	  }
	return "ro";
      case LANG_RUSSIAN:
	switch (sub)
	  {
	  case SUBLANG_DEFAULT: return "ru_RU";
	  }
	return "ru"; /* Ambiguous: could be "ru_RU" or "ru_UA" or "ru_MD".  */
      case LANG_SAAMI: /* actually Northern Sami */ return "se_NO";
      case LANG_SANSKRIT: return "sa_IN";
      case LANG_SINDHI:
	switch (sub)
	  {
	  case SUBLANG_SINDHI_PAKISTAN: return "sd_PK";
	  case SUBLANG_SINDHI_AFGHANISTAN: return "sd_AF";
	  }
	return "sd";
      case LANG_SINHALESE: return "si_LK";
      case LANG_SLOVAK: return "sk_SK";
      case LANG_SLOVENIAN: return "sl_SI";
      case LANG_SOMALI: return "so_SO";
      case LANG_SORBIAN:
	/* FIXME: Adjust this when such locales appear on Unix.  */
	return "wen_DE";
      case LANG_SPANISH:
	switch (sub)
	  {
	  case SUBLANG_SPANISH: return "es_ES";
	  case SUBLANG_SPANISH_MEXICAN: return "es_MX";
	  case SUBLANG_SPANISH_MODERN:
	    return "es_ES@modern";	/* not seen on Unix */
	  case SUBLANG_SPANISH_GUATEMALA: return "es_GT";
	  case SUBLANG_SPANISH_COSTA_RICA: return "es_CR";
	  case SUBLANG_SPANISH_PANAMA: return "es_PA";
	  case SUBLANG_SPANISH_DOMINICAN_REPUBLIC: return "es_DO";
	  case SUBLANG_SPANISH_VENEZUELA: return "es_VE";
	  case SUBLANG_SPANISH_COLOMBIA: return "es_CO";
	  case SUBLANG_SPANISH_PERU: return "es_PE";
	  case SUBLANG_SPANISH_ARGENTINA: return "es_AR";
	  case SUBLANG_SPANISH_ECUADOR: return "es_EC";
	  case SUBLANG_SPANISH_CHILE: return "es_CL";
	  case SUBLANG_SPANISH_URUGUAY: return "es_UY";
	  case SUBLANG_SPANISH_PARAGUAY: return "es_PY";
	  case SUBLANG_SPANISH_BOLIVIA: return "es_BO";
	  case SUBLANG_SPANISH_EL_SALVADOR: return "es_SV";
	  case SUBLANG_SPANISH_HONDURAS: return "es_HN";
	  case SUBLANG_SPANISH_NICARAGUA: return "es_NI";
	  case SUBLANG_SPANISH_PUERTO_RICO: return "es_PR";
	  }
	return "es";
      case LANG_SUTU: return "bnt_TZ"; /* or "st_LS" or "nso_ZA"? */
      case LANG_SWAHILI: return "sw_KE";
      case LANG_SWEDISH:
	switch (sub)
	  {
	  case SUBLANG_DEFAULT: return "sv_SE";
	  case SUBLANG_SWEDISH_FINLAND: return "sv_FI";
	  }
	return "sv";
      case LANG_SYRIAC: return "syr_TR"; /* An extinct language.  */
      case LANG_TAGALOG: return "tl_PH";
      case LANG_TAJIK: return "tg_TJ";
      case LANG_TAMAZIGHT:
	switch (sub)
	  {
	  /* FIXME: Adjust this when Tamazight locales appear on Unix.  */
	  case SUBLANG_TAMAZIGHT_ARABIC: return "ber_MA@arabic";
	  case SUBLANG_TAMAZIGHT_ALGERIA_LATIN: return "ber_DZ@latin";
	  }
	return "ber_MA";
      case LANG_TAMIL:
	switch (sub)
	  {
	  case SUBLANG_DEFAULT: return "ta_IN";
	  }
	return "ta"; /* Ambiguous: could be "ta_IN" or "ta_LK" or "ta_SG".  */
      case LANG_TATAR: return "tt_RU";
      case LANG_TELUGU: return "te_IN";
      case LANG_THAI: return "th_TH";
      case LANG_TIBETAN: return "bo_CN";
      case LANG_TIGRINYA:
	switch (sub)
	  {
	  case SUBLANG_TIGRINYA_ETHIOPIA: return "ti_ET";
	  case SUBLANG_TIGRINYA_ERITREA: return "ti_ER";
	  }
	return "ti";
      case LANG_TSONGA: return "ts_ZA";
      case LANG_TSWANA: return "tn_BW";
      case LANG_TURKISH: return "tr_TR";
      case LANG_TURKMEN: return "tk_TM";
      case LANG_UKRAINIAN: return "uk_UA";
      case LANG_URDU:
	switch (sub)
	  {
	  case SUBLANG_URDU_PAKISTAN: return "ur_PK";
	  case SUBLANG_URDU_INDIA: return "ur_IN";
	  }
	return "ur";
      case LANG_UZBEK:
	switch (sub)
	  {
	  case SUBLANG_UZBEK_LATIN: return "uz_UZ";
	  case SUBLANG_UZBEK_CYRILLIC: return "uz_UZ@cyrillic";
	  }
	return "uz";
      case LANG_VENDA: return "ve_ZA";
      case LANG_VIETNAMESE: return "vi_VN";
      case LANG_WELSH: return "cy_GB";
      case LANG_XHOSA: return "xh_ZA";
      case LANG_YI: return "sit_CN";
      case LANG_YIDDISH: return "yi_IL";
      case LANG_YORUBA: return "yo_NG";
      case LANG_ZULU: return "zu_ZA";
      default: return "C";
      }
  }
# endif
#endif
}

const char *
gl_locale_name (int category, const char *categoryname)
{
  const char *retval;

  retval = gl_locale_name_posix (category, categoryname);
  if (retval != NULL)
    return retval;

  return gl_locale_name_default ();
}
