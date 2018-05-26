// (c) Copyright 2018, Sean Connelly (@voidqk), http://sean.cm
// MIT License
// Project Home: https://github.com/voidqk/basicmidi

#include "basicmidi.h"

// index < 256 for melody, index >= 256 for percussion
// 0xQQRR   QQ = Program Change code, RR = Bank code
static uint16_t patch_midi[] = {
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

const char *bm_patch_str(uint16_t patch){
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
		case BM_PATCH_PERSND_STAN   : return "PSS Standard"                          ;
		case BM_PATCH_PERSND_ROOM   : return "PSS Room"                              ;
		case BM_PATCH_PERSND_POWE   : return "PSS Power"                             ;
		case BM_PATCH_PERSND_ELEC   : return "PSS Electronic"                        ;
		case BM_PATCH_PERSND_ANLG   : return "PSS Analog"                            ;
		case BM_PATCH_PERSND_JAZZ   : return "PSS Jazz"                              ;
		case BM_PATCH_PERSND_BRUS   : return "PSS Brush"                             ;
		case BM_PATCH_PERSND_ORCH   : return "PSS Orchestra"                         ;
		case BM_PATCH_PERSND_SNFX   : return "PSS Sound Effects"                     ;
	}
	return "Invalid patch";
}
