// (c) Copyright 2018, Sean Connelly (@voidqk), http://sean.cm
// MIT License
// Project Home: https://github.com/voidqk/basicmidi

#include "basicmidi.h"
#include <stdarg.h>
#include <stdio.h>

// index < 256 for melody, index >= 256 for percussion
// 0xQQRR   QQ = Program Change code, RR = Bank code
static uint16_t patch_midi[265] = {
	0x0000, 0x0001, 0x0002, 0x0100, 0x0101, 0x0200, 0x0201, 0x0300,
	0x0301, 0x0400, 0x0401, 0x0402, 0x0403, 0x0500, 0x0501, 0x0502,
	0x0503, 0x0504, 0x0600, 0x0601, 0x0602, 0x0603, 0x0700, 0x0701,
	0x0800, 0x0900, 0x0A00, 0x0B00, 0x0B01, 0x0C00, 0x0C01, 0x0D00,
	0x0E00, 0x0E01, 0x0E02, 0x0F00, 0x1000, 0x1001, 0x1002, 0x1003,
	0x1100, 0x1101, 0x1102, 0x1200, 0x1300, 0x1301, 0x1302, 0x1400,
	0x1401, 0x1500, 0x1501, 0x1600, 0x1700, 0x1800, 0x1801, 0x1802,
	0x1803, 0x1900, 0x1901, 0x1902, 0x1903, 0x1A00, 0x1A01, 0x1B00,
	0x1B01, 0x1B02, 0x1C00, 0x1C01, 0x1C02, 0x1C03, 0x1D00, 0x1D01,
	0x1E00, 0x1E01, 0x1E02, 0x1F00, 0x1F01, 0x2000, 0x2100, 0x2101,
	0x2200, 0x2300, 0x2400, 0x2500, 0x2600, 0x2601, 0x2602, 0x2603,
	0x2604, 0x2700, 0x2701, 0x2702, 0x2703, 0x2800, 0x2801, 0x2900,
	0x2A00, 0x2B00, 0x2C00, 0x2D00, 0x2E00, 0x2E01, 0x2F00, 0x3000,
	0x3001, 0x3002, 0x3100, 0x3200, 0x3201, 0x3300, 0x3400, 0x3401,
	0x3500, 0x3501, 0x3600, 0x3601, 0x3700, 0x3701, 0x3702, 0x3703,
	0x3800, 0x3801, 0x3900, 0x3901, 0x3902, 0x3A00, 0x3B00, 0x3B01,
	0x3C00, 0x3C01, 0x3D00, 0x3D01, 0x3E00, 0x3E01, 0x3E02, 0x3E03,
	0x3F00, 0x3F01, 0x3F02, 0x4000, 0x4100, 0x4200, 0x4300, 0x4400,
	0x4500, 0x4600, 0x4700, 0x4800, 0x4900, 0x4A00, 0x4B00, 0x4C00,
	0x4D00, 0x4E00, 0x4F00, 0x5000, 0x5001, 0x5002, 0x5100, 0x5101,
	0x5102, 0x5103, 0x5104, 0x5200, 0x5300, 0x5400, 0x5401, 0x5500,
	0x5600, 0x5700, 0x5701, 0x5800, 0x5900, 0x5901, 0x5A00, 0x5B00,
	0x5B01, 0x5C00, 0x5D00, 0x5E00, 0x5F00, 0x6000, 0x6100, 0x6200,
	0x6201, 0x6300, 0x6400, 0x6500, 0x6600, 0x6601, 0x6602, 0x6700,
	0x6000, 0x6001, 0x6100, 0x6200, 0x6300, 0x6301, 0x6400, 0x6500,
	0x6600, 0x6700, 0x7000, 0x7100, 0x7200, 0x7300, 0x7301, 0x7400,
	0x7401, 0x7500, 0x7501, 0x7600, 0x7601, 0x7602, 0x7700, 0x7800,
	0x7801, 0x7802, 0x7900, 0x7901, 0x7A00, 0x7A01, 0x7A02, 0x7A03,
	0x7A04, 0x7A05, 0x7B00, 0x7B01, 0x7B02, 0x7B03, 0x7C00, 0x7C01,
	0x7C02, 0x7C03, 0x7C04, 0x7C05, 0x7D00, 0x7D01, 0x7D02, 0x7D03,
	0x7D04, 0x7D05, 0x7D06, 0x7D07, 0x7D08, 0x7D09, 0x7E00, 0x7E01,
	0x7E02, 0x7E03, 0x7E04, 0x7E05, 0x7F00, 0x7F01, 0x7F02, 0x7F03,
	0x0000, 0x0800, 0x1000, 0x1800, 0x1900, 0x2000, 0x2800, 0x3000,
	0x3800
};

const char *bm_patchstr(uint16_t patch){
	switch (patch){
		case BM_PATCH_PIANO_ACGR    : return "Acoustic Grand Piano"                  ;
		case BM_PATCH_PIANO_ACGR_WI : return "Acoustic Grand Piano (wide)"           ;
		case BM_PATCH_PIANO_ACGR_DK : return "Acoustic Grand Piano (dark)"           ;
		case BM_PATCH_PIANO_BRAC    : return "Bright Acoustic Piano"                 ;
		case BM_PATCH_PIANO_BRAC_WI : return "Bright Acoustic Piano (wide)"          ;
		case BM_PATCH_PIANO_ELGR    : return "Electric Grand Piano"                  ;
		case BM_PATCH_PIANO_ELGR_WI : return "Electric Grand Piano (wide)"           ;
		case BM_PATCH_PIANO_HOTO    : return "Honky-tonk Piano"                      ;
		case BM_PATCH_PIANO_HOTO_WI : return "Honky-tonk Piano (wide)"               ;
		case BM_PATCH_PIANO_ELE1    : return "Electric Piano 1"                      ;
		case BM_PATCH_PIANO_ELE1_DT : return "Electric Piano 1 (detuned)"            ;
		case BM_PATCH_PIANO_ELE1_VM : return "Electric Piano 1 (velocity mix)"       ;
		case BM_PATCH_PIANO_ELE1_60 : return "Electric Piano 1 (60's)"               ;
		case BM_PATCH_PIANO_ELE2    : return "Electric Piano 2"                      ;
		case BM_PATCH_PIANO_ELE2_DT : return "Electric Piano 2 (detuned)"            ;
		case BM_PATCH_PIANO_ELE2_VM : return "Electric Piano 2 (velocity mix)"       ;
		case BM_PATCH_PIANO_ELE2_LE : return "Electric Piano 2 (legend)"             ;
		case BM_PATCH_PIANO_ELE2_PH : return "Electric Piano 2 (phase)"              ;
		case BM_PATCH_PIANO_HARP    : return "Harpsichord"                           ;
		case BM_PATCH_PIANO_HARP_OM : return "Harpsichord (octave mix)"              ;
		case BM_PATCH_PIANO_HARP_WI : return "Harpsichord (wide)"                    ;
		case BM_PATCH_PIANO_HARP_KO : return "Harpsichord (with key off)"            ;
		case BM_PATCH_PIANO_CLAV    : return "Clavi"                                 ;
		case BM_PATCH_PIANO_CLAV_PU : return "Clavi (pulse)"                         ;
		case BM_PATCH_CHROM_CELE    : return "Celesta"                               ;
		case BM_PATCH_CHROM_GLOC    : return "Glockenspiel"                          ;
		case BM_PATCH_CHROM_MUBO    : return "Music Box"                             ;
		case BM_PATCH_CHROM_VIPH    : return "Vibraphone"                            ;
		case BM_PATCH_CHROM_VIPH_WI : return "Vibraphone (wide)"                     ;
		case BM_PATCH_CHROM_MARI    : return "Marimba"                               ;
		case BM_PATCH_CHROM_MARI_WI : return "Marimba (wide)"                        ;
		case BM_PATCH_CHROM_XYLO    : return "Xylophone"                             ;
		case BM_PATCH_CHROM_BELL_TU : return "Tubular Bells"                         ;
		case BM_PATCH_CHROM_BELL_CH : return "Tubular Bells (church)"                ;
		case BM_PATCH_CHROM_BELL_CA : return "Tubular Bells (carillon)"              ;
		case BM_PATCH_CHROM_DULC    : return "Dulcimer"                              ;
		case BM_PATCH_ORGAN_DRAW    : return "Drawbar Organ"                         ;
		case BM_PATCH_ORGAN_DRAW_DT : return "Drawbar Organ (detuned)"               ;
		case BM_PATCH_ORGAN_DRAW_60 : return "Drawbar Organ (60's)"                  ;
		case BM_PATCH_ORGAN_DRAW_AL : return "Drawbar Organ (alternative)"           ;
		case BM_PATCH_ORGAN_PERC    : return "Percussive Organ"                      ;
		case BM_PATCH_ORGAN_PERC_DT : return "Percussive Organ (detuned)"            ;
		case BM_PATCH_ORGAN_PERC_2  : return "Percussive Organ 2"                    ;
		case BM_PATCH_ORGAN_ROCK    : return "Rock Organ"                            ;
		case BM_PATCH_ORGAN_CHUR    : return "Church Organ"                          ;
		case BM_PATCH_ORGAN_CHUR_OM : return "Church Organ (octave mix)"             ;
		case BM_PATCH_ORGAN_CHUR_DT : return "Church Organ (detuned)"                ;
		case BM_PATCH_ORGAN_REED    : return "Reed Organ"                            ;
		case BM_PATCH_ORGAN_REED_PU : return "Reed Organ (puff)"                     ;
		case BM_PATCH_ORGAN_ACCO    : return "Accordion"                             ;
		case BM_PATCH_ORGAN_ACCO_2  : return "Accordion (alternative)"               ;
		case BM_PATCH_ORGAN_HARM    : return "Harmonica"                             ;
		case BM_PATCH_ORGAN_TANG    : return "Tango Accordion"                       ;
		case BM_PATCH_GUITAR_NYLO   : return "Nylon Acoustic Guitar"                 ;
		case BM_PATCH_GUITAR_NYLO_UK: return "Nylon Acoustic Guitar (ukulele)"       ;
		case BM_PATCH_GUITAR_NYLO_KO: return "Nylon Acoustic Guitar (key off)"       ;
		case BM_PATCH_GUITAR_NYLO_AL: return "Nylon Acoustic Guitar (alternative)"   ;
		case BM_PATCH_GUITAR_STEE   : return "Steel Acoustic Guitar"                 ;
		case BM_PATCH_GUITAR_STEE_12: return "Steel Acoustic Guitar (12-string)"     ;
		case BM_PATCH_GUITAR_STEE_MA: return "Steel Acoustic Guitar (mandolin)"      ;
		case BM_PATCH_GUITAR_STEE_BS: return "Steel Acoustic Guitar (body sound)"    ;
		case BM_PATCH_GUITAR_JAZZ   : return "Jazz Electric Guitar"                  ;
		case BM_PATCH_GUITAR_JAZZ_PS: return "Jazz Electric Guitar (pedal steel)"    ;
		case BM_PATCH_GUITAR_CLEA   : return "Clean Electric Guitar"                 ;
		case BM_PATCH_GUITAR_CLEA_DT: return "Clean Electric Guitar (detuned)"       ;
		case BM_PATCH_GUITAR_CLEA_MT: return "Clean Electric Guitar (midtone)"       ;
		case BM_PATCH_GUITAR_MUTE   : return "Muted Electric Guitar"                 ;
		case BM_PATCH_GUITAR_MUTE_FC: return "Muted Electric Guitar (funky cutting)" ;
		case BM_PATCH_GUITAR_MUTE_VS: return "Muted Electric Guitar (velo-sw)"       ;
		case BM_PATCH_GUITAR_MUTE_JM: return "Muted Electric Guitar (jazz man)"      ;
		case BM_PATCH_GUITAR_OVER   : return "Overdriven Guitar"                     ;
		case BM_PATCH_GUITAR_OVER_PI: return "Overdriven Guitar (pinch)"             ;
		case BM_PATCH_GUITAR_DIST   : return "Distortion Guitar"                     ;
		case BM_PATCH_GUITAR_DIST_FB: return "Distortion Guitar (feedback)"          ;
		case BM_PATCH_GUITAR_DIST_RH: return "Distortion Guitar (rhythm)"            ;
		case BM_PATCH_GUITAR_HARM   : return "Guitar Harmonics"                      ;
		case BM_PATCH_GUITAR_HARM_FB: return "Guitar Harmonics (feedback)"           ;
		case BM_PATCH_BASS_ACOU     : return "Acoustic Bass"                         ;
		case BM_PATCH_BASS_FING     : return "Finger Electric Bass"                  ;
		case BM_PATCH_BASS_FING_SL  : return "Finger Electric Bass (slap)"           ;
		case BM_PATCH_BASS_PICK     : return "Pick Electric Bass"                    ;
		case BM_PATCH_BASS_FRET     : return "Fretless Bass"                         ;
		case BM_PATCH_BASS_SLP1     : return "Slap Bass 1"                           ;
		case BM_PATCH_BASS_SLP2     : return "Slap Bass 2"                           ;
		case BM_PATCH_BASS_SYN1     : return "Synth Bass 1"                          ;
		case BM_PATCH_BASS_SYN1_WA  : return "Synth Bass 1 (warm)"                   ;
		case BM_PATCH_BASS_SYN1_RE  : return "Synth Bass 1 (resonance)"              ;
		case BM_PATCH_BASS_SYN1_CL  : return "Synth Bass 1 (clavi)"                  ;
		case BM_PATCH_BASS_SYN1_HA  : return "Synth Bass 1 (hammer)"                 ;
		case BM_PATCH_BASS_SYN2     : return "Synth Bass 2"                          ;
		case BM_PATCH_BASS_SYN2_AT  : return "Synth Bass 2 (attack)"                 ;
		case BM_PATCH_BASS_SYN2_RU  : return "Synth Bass 2 (rubber)"                 ;
		case BM_PATCH_BASS_SYN2_AP  : return "Synth Bass 2 (attack pulse)"           ;
		case BM_PATCH_STRING_VILN   : return "Violin"                                ;
		case BM_PATCH_STRING_VILN_SA: return "Violin (slow attack)"                  ;
		case BM_PATCH_STRING_VILA   : return "Viola"                                 ;
		case BM_PATCH_STRING_CELL   : return "Cello"                                 ;
		case BM_PATCH_STRING_CONT   : return "Contrabass"                            ;
		case BM_PATCH_STRING_TREM   : return "Tremolo Strings"                       ;
		case BM_PATCH_STRING_PIZZ   : return "Pizzicato Strings"                     ;
		case BM_PATCH_STRING_HARP   : return "Orchestral Harp"                       ;
		case BM_PATCH_STRING_HARP_YC: return "Orchestral Harp (yang chin)"           ;
		case BM_PATCH_STRING_TIMP   : return "Timpani"                               ;
		case BM_PATCH_ENSEM_STR1    : return "String Ensembles 1"                    ;
		case BM_PATCH_ENSEM_STR1_SB : return "String Ensembles 1 (strings and brass)";
		case BM_PATCH_ENSEM_STR1_60 : return "String Ensembles 1 (60s strings)"      ;
		case BM_PATCH_ENSEM_STR2    : return "String Ensembles 2"                    ;
		case BM_PATCH_ENSEM_SYN1    : return "SynthStrings 1"                        ;
		case BM_PATCH_ENSEM_SYN1_AL : return "SynthStrings 1 (alternative)"          ;
		case BM_PATCH_ENSEM_SYN2    : return "SynthStrings 2"                        ;
		case BM_PATCH_ENSEM_CHOI    : return "Choir Aahs"                            ;
		case BM_PATCH_ENSEM_CHOI_AL : return "Choir Aahs (alternative)"              ;
		case BM_PATCH_ENSEM_VOIC    : return "Voice Oohs"                            ;
		case BM_PATCH_ENSEM_VOIC_HM : return "Voice Oohs (humming)"                  ;
		case BM_PATCH_ENSEM_SYVO    : return "Synth Voice"                           ;
		case BM_PATCH_ENSEM_SYVO_AN : return "Synth Voice (analog)"                  ;
		case BM_PATCH_ENSEM_ORHI    : return "Orchestra Hit"                         ;
		case BM_PATCH_ENSEM_ORHI_BP : return "Orchestra Hit (bass hit plus)"         ;
		case BM_PATCH_ENSEM_ORHI_6  : return "Orchestra Hit (6th)"                   ;
		case BM_PATCH_ENSEM_ORHI_EU : return "Orchestra Hit (euro)"                  ;
		case BM_PATCH_BRASS_TRUM    : return "Trumpet"                               ;
		case BM_PATCH_BRASS_TRUM_DS : return "Trumpet (dark soft)"                   ;
		case BM_PATCH_BRASS_TROM    : return "Trombone"                              ;
		case BM_PATCH_BRASS_TROM_AL : return "Trombone (alternative)"                ;
		case BM_PATCH_BRASS_TROM_BR : return "Trombone (bright)"                     ;
		case BM_PATCH_BRASS_TUBA    : return "Tuba"                                  ;
		case BM_PATCH_BRASS_MUTR    : return "Muted Trumpet"                         ;
		case BM_PATCH_BRASS_MUTR_AL : return "Muted Trumpet (alternative)"           ;
		case BM_PATCH_BRASS_FRHO    : return "French Horn"                           ;
		case BM_PATCH_BRASS_FRHO_WA : return "French Horn (warm)"                    ;
		case BM_PATCH_BRASS_BRSE    : return "Brass Section"                         ;
		case BM_PATCH_BRASS_BRSE_OM : return "Brass Section (octave mix)"            ;
		case BM_PATCH_BRASS_SBR1    : return "Synth Brass 1"                         ;
		case BM_PATCH_BRASS_SBR1_AL : return "Synth Brass 1 (alternative)"           ;
		case BM_PATCH_BRASS_SBR1_AN : return "Synth Brass 1 (analog)"                ;
		case BM_PATCH_BRASS_SBR1_JU : return "Synth Brass 1 (jump)"                  ;
		case BM_PATCH_BRASS_SBR2    : return "Synth Brass 2"                         ;
		case BM_PATCH_BRASS_SBR2_AL : return "Synth Brass 2 (alternative)"           ;
		case BM_PATCH_BRASS_SBR2_AN : return "Synth Brass 2 (analog)"                ;
		case BM_PATCH_REED_SOSA     : return "Soprano Sax"                           ;
		case BM_PATCH_REED_ALSA     : return "Alto Sax"                              ;
		case BM_PATCH_REED_TESA     : return "Tenor Sax"                             ;
		case BM_PATCH_REED_BASA     : return "Baritone Sax"                          ;
		case BM_PATCH_REED_OBOE     : return "Oboe"                                  ;
		case BM_PATCH_REED_ENHO     : return "English Horn"                          ;
		case BM_PATCH_REED_BASS     : return "Bassoon"                               ;
		case BM_PATCH_REED_CLAR     : return "Clarinet"                              ;
		case BM_PATCH_PIPE_PICC     : return "Piccolo"                               ;
		case BM_PATCH_PIPE_FLUT     : return "Flute"                                 ;
		case BM_PATCH_PIPE_RECO     : return "Recorder"                              ;
		case BM_PATCH_PIPE_PAFL     : return "Pan Flute"                             ;
		case BM_PATCH_PIPE_BLBO     : return "Blown Bottle"                          ;
		case BM_PATCH_PIPE_SHAK     : return "Shakuhachi"                            ;
		case BM_PATCH_PIPE_WHIS     : return "Whistle"                               ;
		case BM_PATCH_PIPE_OCAR     : return "Ocarina"                               ;
		case BM_PATCH_LEAD_OSC1     : return "Oscilliator 1"                         ;
		case BM_PATCH_LEAD_OSC1_SQ  : return "Oscilliator 1 (square)"                ;
		case BM_PATCH_LEAD_OSC1_SI  : return "Oscilliator 1 (sine)"                  ;
		case BM_PATCH_LEAD_OSC2     : return "Oscilliator 2"                         ;
		case BM_PATCH_LEAD_OSC2_SA  : return "Oscilliator 2 (sawtooth)"              ;
		case BM_PATCH_LEAD_OSC2_SP  : return "Oscilliator 2 (saw + pulse)"           ;
		case BM_PATCH_LEAD_OSC2_DS  : return "Oscilliator 2 (double sawtooth)"       ;
		case BM_PATCH_LEAD_OSC2_AN  : return "Oscilliator 2 (sequenced analog)"      ;
		case BM_PATCH_LEAD_CALL     : return "Calliope"                              ;
		case BM_PATCH_LEAD_CHIF     : return "Chiff"                                 ;
		case BM_PATCH_LEAD_CHAR     : return "Charang"                               ;
		case BM_PATCH_LEAD_CHAR_WL  : return "Charang (wire lead)"                   ;
		case BM_PATCH_LEAD_VOIC     : return "Voice"                                 ;
		case BM_PATCH_LEAD_FIFT     : return "Fifths"                                ;
		case BM_PATCH_LEAD_BALE     : return "Bass + Lead"                           ;
		case BM_PATCH_LEAD_BALE_SW  : return "Bass + Lead (soft wrl)"                ;
		case BM_PATCH_PAD_NEAG      : return "New Age"                               ;
		case BM_PATCH_PAD_WARM      : return "Warm"                                  ;
		case BM_PATCH_PAD_WARM_SI   : return "Warm (sine)"                           ;
		case BM_PATCH_PAD_POLY      : return "Polysynth"                             ;
		case BM_PATCH_PAD_CHOI      : return "Choir"                                 ;
		case BM_PATCH_PAD_CHOI_IT   : return "Choir (itopia)"                        ;
		case BM_PATCH_PAD_BOWE      : return "Bowed"                                 ;
		case BM_PATCH_PAD_META      : return "Metallic"                              ;
		case BM_PATCH_PAD_HALO      : return "Halo"                                  ;
		case BM_PATCH_PAD_SWEE      : return "Sweep"                                 ;
		case BM_PATCH_SFX1_RAIN     : return "Rain"                                  ;
		case BM_PATCH_SFX1_SOTR     : return "Soundtrack"                            ;
		case BM_PATCH_SFX1_CRYS     : return "Crystal"                               ;
		case BM_PATCH_SFX1_CRYS_MA  : return "Crystal (mallet)"                      ;
		case BM_PATCH_SFX1_ATMO     : return "Atmosphere"                            ;
		case BM_PATCH_SFX1_BRIG     : return "Brightness"                            ;
		case BM_PATCH_SFX1_GOBL     : return "Goblins"                               ;
		case BM_PATCH_SFX1_ECHO     : return "Echoes"                                ;
		case BM_PATCH_SFX1_ECHO_BE  : return "Echoes (bell)"                         ;
		case BM_PATCH_SFX1_ECHO_PA  : return "Echoes (pan)"                          ;
		case BM_PATCH_SFX1_SCFI     : return "Sci-Fi"                                ;
		case BM_PATCH_ETHNIC_SITA   : return "Sitar"                                 ;
		case BM_PATCH_ETHNIC_SITA_BE: return "Sitar (bend)"                          ;
		case BM_PATCH_ETHNIC_BANJ   : return "Banjo"                                 ;
		case BM_PATCH_ETHNIC_SHAM   : return "Shamisen"                              ;
		case BM_PATCH_ETHNIC_KOTO   : return "Koto"                                  ;
		case BM_PATCH_ETHNIC_KOTO_TA: return "Koto (taisho)"                         ;
		case BM_PATCH_ETHNIC_KALI   : return "Kalimba"                               ;
		case BM_PATCH_ETHNIC_BAPI   : return "Bag Pipe"                              ;
		case BM_PATCH_ETHNIC_FIDD   : return "Fiddle"                                ;
		case BM_PATCH_ETHNIC_SHAN   : return "Shanai"                                ;
		case BM_PATCH_PERC_TIBE     : return "Tinkle Bell"                           ;
		case BM_PATCH_PERC_AGOG     : return "Agogo"                                 ;
		case BM_PATCH_PERC_STDR     : return "Steel Drums"                           ;
		case BM_PATCH_PERC_WOOD     : return "Woodblock"                             ;
		case BM_PATCH_PERC_WOOD_CA  : return "Woodblock (castanets)"                 ;
		case BM_PATCH_PERC_TADR     : return "Taiko Drum"                            ;
		case BM_PATCH_PERC_TADR_CB  : return "Taiko Drum (concert bass)"             ;
		case BM_PATCH_PERC_METO     : return "Melodic Tom"                           ;
		case BM_PATCH_PERC_METO_PO  : return "Melodic Tom (power)"                   ;
		case BM_PATCH_PERC_SYDR     : return "Synth Drum"                            ;
		case BM_PATCH_PERC_SYDR_RB  : return "Synth Drum (rhythm box tom)"           ;
		case BM_PATCH_PERC_SYDR_EL  : return "Synth Drum (electric)"                 ;
		case BM_PATCH_PERC_RECY     : return "Reverse Cymbal"                        ;
		case BM_PATCH_SFX2_G0_GUFR  : return "Guitar Fret Noise"                     ;
		case BM_PATCH_SFX2_G0_GUCU  : return "Guitar Cutting Noise (GM2)"            ;
		case BM_PATCH_SFX2_G0_STSL  : return "Acoustic Bass String Slap (GM2)"       ;
		case BM_PATCH_SFX2_G1_BRNO  : return "Breath Noise"                          ;
		case BM_PATCH_SFX2_G1_FLKC  : return "Flute Key Click (GM2)"                 ;
		case BM_PATCH_SFX2_G2_SEAS  : return "Seashore"                              ;
		case BM_PATCH_SFX2_G2_RAIN  : return "Rain (GM2)"                            ;
		case BM_PATCH_SFX2_G2_THUN  : return "Thunder (GM2)"                         ;
		case BM_PATCH_SFX2_G2_WIND  : return "Wind (GM2)"                            ;
		case BM_PATCH_SFX2_G2_STRE  : return "Stream (GM2)"                          ;
		case BM_PATCH_SFX2_G2_BUBB  : return "Bubble (GM2)"                          ;
		case BM_PATCH_SFX2_G3_BTW1  : return "Bird Tweet 1"                          ;
		case BM_PATCH_SFX2_G3_DOG   : return "Dog (GM2)"                             ;
		case BM_PATCH_SFX2_G3_HOGA  : return "Horse Gallop (GM2)"                    ;
		case BM_PATCH_SFX2_G3_BTW2  : return "Bird Tweet 2 (GM2)"                    ;
		case BM_PATCH_SFX2_G4_TEL1  : return "Telephone Ring 1"                      ;
		case BM_PATCH_SFX2_G4_TEL2  : return "Telephone Ring 2 (GM2)"                ;
		case BM_PATCH_SFX2_G4_DOCR  : return "Door Creaking (GM2)"                   ;
		case BM_PATCH_SFX2_G4_DOOR  : return "Door (GM2)"                            ;
		case BM_PATCH_SFX2_G4_SCRA  : return "Scratch (GM2)"                         ;
		case BM_PATCH_SFX2_G4_WICH  : return "Wind Chime (GM2)"                      ;
		case BM_PATCH_SFX2_G5_HELI  : return "Helicopter"                            ;
		case BM_PATCH_SFX2_G5_CAEN  : return "Car Engine (GM2)"                      ;
		case BM_PATCH_SFX2_G5_CAST  : return "Car Stop (GM2)"                        ;
		case BM_PATCH_SFX2_G5_CAPA  : return "Car Pass (GM2)"                        ;
		case BM_PATCH_SFX2_G5_CACR  : return "Car Crash (GM2)"                       ;
		case BM_PATCH_SFX2_G5_SIRE  : return "Siren (GM2)"                           ;
		case BM_PATCH_SFX2_G5_TRAI  : return "Train (GM2)"                           ;
		case BM_PATCH_SFX2_G5_JETP  : return "Jetplane (GM2)"                        ;
		case BM_PATCH_SFX2_G5_STAR  : return "Starship (GM2)"                        ;
		case BM_PATCH_SFX2_G5_BUNO  : return "Burst Noise (GM2)"                     ;
		case BM_PATCH_SFX2_G6_APPL  : return "Applause"                              ;
		case BM_PATCH_SFX2_G6_LAUG  : return "Laughing (GM2)"                        ;
		case BM_PATCH_SFX2_G6_SCRE  : return "Screaming (GM2)"                       ;
		case BM_PATCH_SFX2_G6_PUNC  : return "Punch (GM2)"                           ;
		case BM_PATCH_SFX2_G6_HEBE  : return "Heart Beat (GM2)"                      ;
		case BM_PATCH_SFX2_G6_FOOT  : return "Footsteps (GM2)"                       ;
		case BM_PATCH_SFX2_G7_GUSH  : return "Gun Shot"                              ;
		case BM_PATCH_SFX2_G7_MAGU  : return "Machine Gun (GM2)"                     ;
		case BM_PATCH_SFX2_G7_LAGU  : return "Laser Gun (GM2)"                       ;
		case BM_PATCH_SFX2_G7_EXPL  : return "Explosion (GM2)"                       ;
		case BM_PATCH_PERSND_STAN   : return "Percussion Standard"                   ;
		case BM_PATCH_PERSND_ROOM   : return "Percussion Room"                       ;
		case BM_PATCH_PERSND_POWE   : return "Percussion Power"                      ;
		case BM_PATCH_PERSND_ELEC   : return "Percussion Electronic"                 ;
		case BM_PATCH_PERSND_ANLG   : return "Percussion Analog"                     ;
		case BM_PATCH_PERSND_JAZZ   : return "Percussion Jazz"                       ;
		case BM_PATCH_PERSND_BRUS   : return "Percussion Brush"                      ;
		case BM_PATCH_PERSND_ORCH   : return "Percussion Orchestra"                  ;
		case BM_PATCH_PERSND_SNFX   : return "Percussion Sound Effects"              ;
	}
	return "Invalid patch";
}

static void rest_init(bm_state_st *state){
	// TODO: initialize the rest
}

void bm_init(bm_state_st *state){
	state->divisor = 1; // TODO: what is a good default..?
	rest_init(state);
}

void bm_update(bm_state_st *state, bm_ev_st *events, int events_size){
	for (int i = 0; i < events_size; i++){
		switch (events[i].type){
			case BM_EV_RESET:
				if (events[i].u.reset > 0)
					state->divisor = events[i].u.reset;
				rest_init(state);
				break;
			case BM_EV_TEMPO:
				state->tempo = events[i].u.tempo;
				break;
			case BM_EV_MASTVOL:
				state->mastvol = events[i].u.mastvol;
				break;
			case BM_EV_MASTPAN:
				state->mastpan = events[i].u.mastpan;
				break;
			case BM_EV_NOTEON:
				state->channels[events[i].u.noteon.channel].notes[events[i].u.noteon.note] =
					(struct bm_state_note_struct){
						.down = true,
						.velocity = events[i].u.noteon.velocity
					};
				break;
			case BM_EV_NOTEOFF:
				state->channels[events[i].u.noteoff.channel].notes[events[i].u.noteoff.note] =
					(struct bm_state_note_struct){
						.down = false,
						.velocity = 0
					};
				break;
			case BM_EV_PEDALON:
				state->channels[events[i].u.pedalon.channel].pedals[events[i].u.pedalon.pedal] =
					true;
				break;
			case BM_EV_PEDALOFF:
				state->channels[events[i].u.pedaloff.channel].pedals[events[i].u.pedaloff.pedal] =
					false;
				break;
			case BM_EV_CHANVOL:
				state->channels[events[i].u.chanvol.channel].vol = events[i].u.chanvol.vol;
				break;
			case BM_EV_CHANPAN:
				state->channels[events[i].u.chanpan.channel].pan = events[i].u.chanpan.pan;
				break;
			case BM_EV_PATCH:
				state->channels[events[i].u.patch.channel].patch = events[i].u.patch.patch;
				break;
			case BM_EV_BEND:
				state->channels[events[i].u.bend.channel].bend = events[i].u.bend.bend;
				break;
			case BM_EV_MOD:
				state->channels[events[i].u.mod.channel].mod = events[i].u.mod.mod;
				break;
		}
	}
}

void bm_deviceinit(bm_device_st *device){
	device->running_status = -1;
	for (int i = 0; i < 16; i++){
		device->ctrls[i].bank = i == 9 ? 0x7800 : 0x7900;
		device->ctrls[i].vol = 0x3FFF;
		device->ctrls[i].pan = 0x2000;
	}
}

static void warn(bm_warn_f f_warn, void *user, const char *fmt, ...){
	if (f_warn == NULL)
		return;
	va_list args;
	va_start(args, fmt);
	char buf[100];
	vsnprintf(buf, sizeof(buf), fmt, args);
	va_end(args);
	f_warn(buf, user);
}

static inline const char *ss(int num){
	return num == 1 ? "" : "s";
}

static int midi_single(const uint8_t *data, int data_size, bm_device_st *device, bm_warn_f f_warn,
	void *user, bm_ev_st *event_out){
	// read msg
	int p = 0;
	int msg = data[p++];
	if (msg < 0x80){
		// use running status
		if (device->running_status < 0){
			warn(f_warn, user, "Invalid message %02X", msg);
			return p; // consume the bad data
		}
		else{
			msg = device->running_status;
			p--;
		}
	}

	// interpret msg
	if (msg >= 0x80 && msg < 0x90){ // Note-Off
		if (p + 1 >= data_size){
			warn(f_warn, user, "Bad Note-Off message (out of data)");
			return data_size;
		}
		device->running_status = msg;
		uint8_t note = data[p++];
		uint8_t vel = data[p++];
		if (note >= 0x80){
			warn(f_warn, user, "Bad Note-Off message (invalid note %02X)", note);
			note ^= 0x80;
		}
		if (vel >= 0x80){
			warn(f_warn, user, "Bad Note-Off message (invalid velocity %02X)", vel);
			vel ^= 0x80;
		}
		*event_out = (bm_ev_st){
			.type = BM_EV_NOTEOFF,
			.u.noteoff.channel = msg & 0x0F,
			.u.noteoff.note = note
		};
		return p;
	}
	else if (msg >= 0x90 && msg < 0xA0){ // Note On
		if (p + 1 >= data_size){
			warn(f_warn, user, "Bad Note-On message (out of data)");
			return data_size;
		}
		device->running_status = msg;
		uint8_t note = data[p++];
		uint8_t vel = data[p++];
		if (note >= 0x80){
			warn(f_warn, user, "Bad Note-On message (invalid note %02X)", note);
			note ^= 0x80;
		}
		if (vel >= 0x80){
			warn(f_warn, user, "Bad Note-On message (invalid velocity %02X)", vel);
			vel ^= 0x80;
		}
		if (vel == 0){
			*event_out = (bm_ev_st){
				.type = BM_EV_NOTEOFF,
				.u.noteoff.channel = msg & 0x0F,
				.u.noteoff.note = note
			};
			return p;
		}
		*event_out = (bm_ev_st){
			.type = BM_EV_NOTEON,
			.u.noteon.channel = msg & 0x0F,
			.u.noteon.note = note,
			.u.noteon.velocity = vel
		};
		return p;
	}
	else if (msg >= 0xA0 && msg < 0xB0){ // Note Pressure
		if (p + 1 >= data_size){
			warn(f_warn, user, "Bad Note Pressure message (out of data)");
			return data_size;
		}
		device->running_status = msg;
		uint8_t note = data[p++];
		uint8_t pressure = data[p++];
		if (note >= 0x80){
			warn(f_warn, user, "Bad Note Pressure message (invalid note %02X)", note);
			note ^= 0x80;
		}
		if (pressure >= 0x80){
			warn(f_warn, user, "Bad Note Pressure message (invalid pressure %02X)", pressure);
			pressure ^= 0x80;
		}
		return p;
	}
	else if (msg >= 0xB0 && msg < 0xC0){ // Control Change
		if (p + 1 >= data_size){
			warn(f_warn, user, "Bad Control Change message (out of data)");
			return data_size;
		}
		device->running_status = msg;
		uint8_t ctrl = data[p++];
		int val = data[p++];
		if (ctrl >= 0x80){
			warn(f_warn, user, "Bad Control Change message (invalid control %02X)", ctrl);
			ctrl ^= 0x80;
		}
		if (val >= 0x80){
			warn(f_warn, user, "Bad Control Change message (invalid value %02X)", val);
			val ^= 0x80;
		}

		int chan = msg & 0xF;
		if (ctrl == 0x00) // Bank Select MSB
			device->ctrls[chan].bank = 0x100000 | (val << 8);
		else if (ctrl == 0x20) // Bank Select LSB
			device->ctrls[chan].bank = (device->ctrls[chan].bank & 0xF0FF00) | 0x010000 | val;
		else if (ctrl == 0x07 || ctrl == 0x27){ // Channel Volume
			if (ctrl == 0x07) // MSB
				device->ctrls[chan].vol = val << 7;
			else // LSB
				device->ctrls[chan].vol = (device->ctrls[chan].vol & 0x3F80) | val;
			*event_out = (bm_ev_st){
				.type = BM_EV_CHANVOL,
				.u.chanvol.channel = chan,
				.u.chanvol.vol = device->ctrls[chan].vol
			};
		}
		else if (ctrl == 0x0A || ctrl == 0x2A){ // Channel Pan
			if (ctrl == 0x0A) // MSB
				device->ctrls[chan].pan = val << 7;
			else // LSB
				device->ctrls[chan].pan = (device->ctrls[chan].pan & 0x3F80) | val;
			*event_out = (bm_ev_st){
				.type = BM_EV_CHANPAN,
				.u.chanpan.channel = chan,
				.u.chanpan.pan = device->ctrls[chan].pan - 0x2000
			};
		}
		return p;
	}
	else if (msg >= 0xC0 && msg < 0xD0){ // Program Change
		if (p >= data_size){
			warn(f_warn, user, "Bad Program Change message (out of data)");
			return data_size;
		}
		device->running_status = msg;
		uint8_t patch = data[p++];
		if (patch >= 0x80){
			warn(f_warn, user, "Bad Program Change message (invalid patch %02X)", patch);
			patch ^= 0x80;
		}
		int chan = msg & 0xF;
		int bank = device->ctrls[chan].bank;
		if ((bank & 0x110000) != 0x110000)
			warn(f_warn, user, "Incomplete bank");

		bool melody = (bank & 0xFF00) == 0x7900;
		bool percussion = (bank & 0xFF00) == 0x7800;
		if (melody || percussion){
			// calculate patch based on format of patch_midi
			patch = (patch << 8) | (bank & 0xFF);
			int start = melody ? 0 : 256;
			int end = melody ? 256 : 265;
			for (int i = start; i < end; i++){
				if (patch_midi[i] == patch){
					*event_out = (bm_ev_st){
						.type = BM_EV_PATCH,
						.u.patch.channel = chan,
						.u.patch.patch = i
					};
					return p;
				}
			}
			warn(f_warn, user, "Unknown patch %02X for bank %04X", patch, bank);
		}
		else
			warn(f_warn, user, "Unknown bank %04X", bank);
		return p;
	}
	else if (msg >= 0xD0 && msg < 0xE0){ // Channel Pressure
		if (p >= data_size){
			warn(f_warn, user, "Bad Channel Pressure message (out of data)");
			return data_size;
		}
		device->running_status = msg;
		uint8_t pressure = data[p++];
		if (pressure >= 0x80)
			warn(f_warn, user, "Bad Channel Pressure message (invalid pressure %02X)", pressure);
		return p;
	}
	else if (msg >= 0xE0 && msg < 0xF0){ // Pitch Bend
		if (p + 1 >= data_size){
			warn(f_warn, user, "Bad Pitch Bend message (out of data)");
			return data_size;
		}
		device->running_status = msg;
		int p1 = data[p++];
		int p2 = data[p++];
		if (p1 >= 0x80){
			warn(f_warn, user, "Bad Pitch Bend message (invalid lower bits %02X)", p1);
			p1 ^= 0x80;
		}
		if (p2 >= 0x80){
			warn(f_warn, user, "Bad Pitch Bend message (invalid higher bits %02X)", p2);
			p2 ^= 0x80;
		}
		int chan = msg & 0xF;
		int bend = p1 | (p2 << 7);
		*event_out = (bm_ev_st){
			.type = BM_EV_BEND,
			.u.bend.channel = chan,
			.u.bend.bend = bend - 0x1000
		};
		return p;
	}
	else if (msg == 0xF0 || msg == 0xF7){ // SysEx Event
		device->running_status = -1; // TODO: validate we should clear this
		// read length as a variable int
		int dl = 0;
		int len = 0;
		while (true){
			if (p >= data_size){
				warn(f_warn, user, "Bad SysEx Event (out of data)");
				return data_size;
			}
			len++;
			if (len >= 5){
				warn(f_warn, user, "Bad SysEx Event (invalid data length)");
				return 1; // consume the message
			}
			int t = data[p++];
			dl = (dl << 7) | (t & 0x7F);
			if ((t & 0x80) == 0)
				break;
		}
		if (p + dl > data_size){
			warn(f_warn, user, "Bad SysEx Event (data length too large)");
			return data_size;
		}
		if (dl == 7 &&
			data[p + 0] == 0x7F &&
			data[p + 2] == 0x04 &&
			data[p + 7] == 0xF7){ // SysEx Real Time Device Control
			if (data[p + 3] == 0x01){ // Master Volume
				int v = (((int)(data[p + 5] & 0x7F)) << 7) | (data[p + 4] & 0x7F);
				*event_out = (bm_ev_st){
					.type = BM_EV_MASTVOL,
					.u.mastvol = v
				};
			}
			else if (data[p + 3] == 0x02){ // Master Balance
				int v = (((int)(data[p + 5] & 0x7F)) << 7) | (data[p + 4] & 0x7F);
				*event_out = (bm_ev_st){
					.type = BM_EV_MASTPAN,
					.u.mastpan = v - 0x1000
				};
			}
		}
		return p + dl;
	}
	else if (msg == 0xFF){ // Meta Event
		device->running_status = -1; // TODO: validate we should clear this
		if (p + 1 >= data_size){
			warn(f_warn, user, "Bad Meta Event (out of data)");
			return data_size;
		}
		int type = data[p++];
		int len = data[p++];
		if (p + len > data_size){
			warn(f_warn, user, "Bad Meta Event (data length too large)");
			return data_size;
		}
		if (type == 0x2F){ // 00  End of Track
			if (len != 0)
				warn(f_warn, user, "Expecting zero-length data for End of Track message");
			if (p < data_size){
				uint64_t pd = data_size - p;
				warn(f_warn, user, "Extra data at end of track: %llu byte%s", pd, ss(pd));
			}
			return data_size;
		}
		else if (type == 0x51){ // 03 TT TT TT  Set Tempo
			if (len < 3)
				warn(f_warn, user, "Missing data for Set Tempo event");
			else{
				if (len > 3)
					warn(f_warn, user, "Extra %d byte%s for Set Tempo event", len - 3, ss(len - 3));
				int tempo = ((int)data[p + 0] << 16) | ((int)data[p + 1] << 8) | data[p + 2];
				if (tempo == 0)
					warn(f_warn, user, "Invalid tempo (0)");
				else{
					*event_out = (bm_ev_st){
						.type = BM_EV_TEMPO,
						.u.tempo = tempo
					};
				}
			}
		}
		return p + len;
	}

	device->running_status = -1;
	warn(f_warn, user, "Unknown message type %02X", msg);
	return 1; // consume the message
}

int bm_devicebytes(bm_device_st *device, const uint8_t *data, int size, bm_ev_st *events_out,
	int max_events_size, bm_warn_f f_warn, void *user){
	int e = 0;
	int p = 0;
	bm_ev_st ev;
	while (e < max_events_size && p < size){
		ev.type = 99; // set event type to something invalid to detect if one is written
		p += midi_single(data, size, device, f_warn, user, &ev);
		if ((int)ev.type != 99)
			events_out[e++] = ev;
	}
	return e;
}

typedef struct {
	int type;
	int data_size;
	int data_start;
	int alignment;
} chunk_st;

static inline int chunk_type(uint8_t b1, uint8_t b2, uint8_t b3, uint8_t b4){
	if (b1 == 'M' && b2 == 'T'){
		if (b3 == 'h' && b4 == 'd')
			return 0; // header
		else if (b3 == 'r' && b4 == 'k')
			return 1; // track
	}
	return -1; // invalid
}

static bool read_chunk(int p, int size, const uint8_t *data, chunk_st *chk){
	if (p + 8 > size)
		return false;
	int type = chunk_type(data[p + 0], data[p + 1], data[p + 2], data[p + 3]);
	int alignment = 0;
	if (type < 0){
		int p_orig = p;
		// rewind 7 bytes and search forward until end of data
		p = p < 7 ? 0 : p - 7;
		while (p + 4 <= size){
			type = chunk_type(data[p + 0], data[p + 1], data[p + 2], data[p + 3]);
			if (type >= 0)
				break;
			p++;
		}
		if (type >= 0)
			alignment = p - p_orig;
	}
	if (type < 0 || p + 8 > size)
		return false;
	if (data[p + 4] > 0) // if size is claiming something more than 16 megs, just bail
		return false;
	chk->type = type;
	chk->data_size =
		((int)data[p + 5] << 16) |
		((int)data[p + 6] <<  8) |
		((int)data[p + 7]);
	chk->data_start = p + 8;
	chk->alignment = alignment;
	return true;
}

void bm_readmidi(const uint8_t *data, int size, bm_event_f f_event, bm_warn_f f_warn, void *user){
	if (size < 14 ||
		data[0] != 'M' || data[1] != 'T' || data[2] != 'h' || data[3] != 'd' ||
		data[4] !=  0  || data[5] !=  0  || data[6] !=  0  || data[7] < 6){
		warn(f_warn, user, "Invalid header");
		return;
	}
	int pos = 0;
	bool found_header = false;
	int hd_format;
	int hd_track_ch;
	int track_i = 0;
	bm_device_st device;
	chunk_st chk;
	while (pos < size){
		if (!read_chunk(pos, size, data, &chk)){
			if (!found_header)
				warn(f_warn, user, "Invalid header");
			else{
				int dif = size - pos;
				warn(f_warn, user, "Unrecognized data (%d byte%s) at end of file", dif, ss(dif));
			}
			return;
		}
		if (chk.alignment != 0)
			warn(f_warn, user, "Chunk misaligned by %d byte%s", chk.alignment, ss(chk.alignment));
		int orig_size = chk.data_size;
		if (chk.data_start + chk.data_size > size){
			int offset = chk.data_start + chk.data_size - size;
			warn(f_warn, user, "Chunk ends %d byte%s too early", offset, ss(offset));
			chk.data_size -= offset;
		}
		pos = chk.data_start + chk.data_size;
		switch (chk.type){
			case 0: { // MThd
				if (found_header)
					warn(f_warn, user, "Multiple header chunks present");
				found_header = true;
				if (orig_size != 6){
					warn(f_warn, user,
						"Header chunk has non-standard size %d byte%s (expecting 6 bytes)",
						orig_size, ss(orig_size));
				}
				if (chk.data_size >= 2){
					hd_format = ((int)data[chk.data_start + 0] << 8) | data[chk.data_start + 1];
					if (hd_format != 0 && hd_format != 1 && hd_format != 2){
						warn(f_warn, user, "Header reports bad format (%d)", hd_format);
						hd_format = 1;
					}
				}
				else{
					warn(f_warn, user, "Header missing format");
					hd_format = 1;
				}
				if (chk.data_size >= 4){
					hd_track_ch = ((int)data[chk.data_start + 2] << 8) | data[chk.data_start + 3];
					if (hd_format == 0 && hd_track_ch != 1){
						warn(f_warn, user,
							"Format 0 expecting 1 track chunk, header is reporting %d chunks",
							hd_track_ch);
					}
				}
				else{
					warn(f_warn, user, "Header missing track chunk count");
					hd_track_ch = -1;
				}
				int division = 1;
				if (chk.data_size >= 6){
					division = ((int)data[chk.data_start + 4] << 8) | data[chk.data_start + 5];
					if (division & 0x8000){
						warn(f_warn, user, "Unsupported timing format (SMPTE)");
						division = 1;
					}
				}
				else
					warn(f_warn, user, "Header missing division");
				f_event((bm_delta_ev_st){
					.delta = 0,
					.ev = (bm_ev_st){
						.type = BM_EV_RESET,
						.u.reset = division
					}
				}, user);
			} break;

			case 1: { // MTrk
				if (hd_format == 0 && track_i > 0){
					warn(f_warn, user, "Format 0 expecting 1 track chunk, found more than one");
					hd_format = 1;
				}
				bm_deviceinit(&device);
				int p = chk.data_start;
				int p_end = chk.data_start + chk.data_size;
				while (p < p_end){
					// read delta as variable int
					int dt = 0;
					int len = 0;
					while (true){
						len++;
						if (len >= 5){
							warn(f_warn, user, "Invalid timestamp in track %d", track_i);
							goto mtrk_end;
						}
						int t = data[p++];
						if (t & 0x80){
							if (p >= p_end){
								warn(f_warn, user, "Invalid timestamp in track %d", track_i);
								goto mtrk_end;
							}
							dt = (dt << 7) | (t & 0x7F);
						}
						else{
							dt = (dt << 7) | t;
							break;
						}
					}

					if (p >= p_end)
						warn(f_warn, user, "Missing message");
					else{
						// create an event with an invalid type, in order to detect if
						// midi_single writes out an event
						bm_delta_ev_st dev = { .delta = dt, .ev = { .type = 99 } };
						p += midi_single(&data[p], p_end - p, &device, f_warn, user, &dev.ev);
						if ((int)dev.ev.type != 99)
							f_event(dev, user);
					}
				}
				warn(f_warn, user, "Track %d ended before receiving End of Track message", track_i);
				mtrk_end:
				track_i++;
			} break;
		}
	}
	if (found_header && hd_track_ch != track_i){
		warn(f_warn, user, "Mismatch between reported track count (%d) and actual track "
			"count (%d)", hd_track_ch, track_i);
	}
	return;
}

void bm_writemidi(bm_delta_ev_st *events, int size, bm_dump_f f_dump, void *user){
	// TODO: this
}
