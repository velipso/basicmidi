// (c) Copyright 2018, Sean Connelly (@voidqk), http://sean.cm
// MIT License
// Project Home: https://github.com/voidqk/basicmidi

#ifndef BASICMIDI__H
#define BASICMIDI__H

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

typedef enum {
	BM_EV_RESET,    // reset all sound and optionally set ticks per quarter-note
	BM_EV_TEMPO,    // set microseconds per quarter-note
	BM_EV_MASTVOL,  // master volume
	BM_EV_MASTPAN,  // master panning
	BM_EV_NOTEON,   // note on
	BM_EV_NOTEOFF,  // note off
	BM_EV_PEDALON,  // foot pedal on
	BM_EV_PEDALOFF, // foot pedal off
	BM_EV_CHANVOL,  // channel volume
	BM_EV_CHANPAN,  // channel panning
	BM_EV_PATCH,    // channel patch
	BM_EV_BEND,     // channel pitch bend
	BM_EV_MOD       // channel mod wheel
} bm_ev_type;

#define BM_PEDAL_DAMPER           0
#define BM_PEDAL_PORTAMENTO       1
#define BM_PEDAL_SOSTENUTO        2
#define BM_PEDAL_SOFT             3
#define BM_PEDAL_LEGATO           4
#define BM_PEDAL_HOLD             5

#define BM_PATCH_PIANO_ACGR       0
#define BM_PATCH_PIANO_ACGR_WI    1
#define BM_PATCH_PIANO_ACGR_DK    2
#define BM_PATCH_PIANO_BRAC       3
#define BM_PATCH_PIANO_BRAC_WI    4
#define BM_PATCH_PIANO_ELGR       5
#define BM_PATCH_PIANO_ELGR_WI    6
#define BM_PATCH_PIANO_HOTO       7
#define BM_PATCH_PIANO_HOTO_WI    8
#define BM_PATCH_PIANO_ELE1       9
#define BM_PATCH_PIANO_ELE1_DT   10
#define BM_PATCH_PIANO_ELE1_VM   11
#define BM_PATCH_PIANO_ELE1_60   12
#define BM_PATCH_PIANO_ELE2      13
#define BM_PATCH_PIANO_ELE2_DT   14
#define BM_PATCH_PIANO_ELE2_VM   15
#define BM_PATCH_PIANO_ELE2_LE   16
#define BM_PATCH_PIANO_ELE2_PH   17
#define BM_PATCH_PIANO_HARP      18
#define BM_PATCH_PIANO_HARP_OM   19
#define BM_PATCH_PIANO_HARP_WI   20
#define BM_PATCH_PIANO_HARP_KO   21
#define BM_PATCH_PIANO_CLAV      22
#define BM_PATCH_PIANO_CLAV_PU   23
#define BM_PATCH_CHROM_CELE      24
#define BM_PATCH_CHROM_GLOC      25
#define BM_PATCH_CHROM_MUBO      26
#define BM_PATCH_CHROM_VIPH      27
#define BM_PATCH_CHROM_VIPH_WI   28
#define BM_PATCH_CHROM_MARI      29
#define BM_PATCH_CHROM_MARI_WI   30
#define BM_PATCH_CHROM_XYLO      31
#define BM_PATCH_CHROM_BELL_TU   32
#define BM_PATCH_CHROM_BELL_CH   33
#define BM_PATCH_CHROM_BELL_CA   34
#define BM_PATCH_CHROM_DULC      35
#define BM_PATCH_ORGAN_DRAW      36
#define BM_PATCH_ORGAN_DRAW_DT   37
#define BM_PATCH_ORGAN_DRAW_60   38
#define BM_PATCH_ORGAN_DRAW_AL   39
#define BM_PATCH_ORGAN_PERC      40
#define BM_PATCH_ORGAN_PERC_DT   41
#define BM_PATCH_ORGAN_PERC_2    42
#define BM_PATCH_ORGAN_ROCK      43
#define BM_PATCH_ORGAN_CHUR      44
#define BM_PATCH_ORGAN_CHUR_OM   45
#define BM_PATCH_ORGAN_CHUR_DT   46
#define BM_PATCH_ORGAN_REED      47
#define BM_PATCH_ORGAN_REED_PU   48
#define BM_PATCH_ORGAN_ACCO      49
#define BM_PATCH_ORGAN_ACCO_2    50
#define BM_PATCH_ORGAN_HARM      51
#define BM_PATCH_ORGAN_TANG      52
#define BM_PATCH_GUITAR_NYLO     53
#define BM_PATCH_GUITAR_NYLO_UK  54
#define BM_PATCH_GUITAR_NYLO_KO  55
#define BM_PATCH_GUITAR_NYLO_AL  56
#define BM_PATCH_GUITAR_STEE     57
#define BM_PATCH_GUITAR_STEE_12  58
#define BM_PATCH_GUITAR_STEE_MA  59
#define BM_PATCH_GUITAR_STEE_BS  60
#define BM_PATCH_GUITAR_JAZZ     61
#define BM_PATCH_GUITAR_JAZZ_PS  62
#define BM_PATCH_GUITAR_CLEA     63
#define BM_PATCH_GUITAR_CLEA_DT  64
#define BM_PATCH_GUITAR_CLEA_MT  65
#define BM_PATCH_GUITAR_MUTE     66
#define BM_PATCH_GUITAR_MUTE_FC  67
#define BM_PATCH_GUITAR_MUTE_VS  68
#define BM_PATCH_GUITAR_MUTE_JM  69
#define BM_PATCH_GUITAR_OVER     70
#define BM_PATCH_GUITAR_OVER_PI  71
#define BM_PATCH_GUITAR_DIST     72
#define BM_PATCH_GUITAR_DIST_FB  73
#define BM_PATCH_GUITAR_DIST_RH  74
#define BM_PATCH_GUITAR_HARM     75
#define BM_PATCH_GUITAR_HARM_FB  76
#define BM_PATCH_BASS_ACOU       77
#define BM_PATCH_BASS_FING       78
#define BM_PATCH_BASS_FING_SL    79
#define BM_PATCH_BASS_PICK       80
#define BM_PATCH_BASS_FRET       81
#define BM_PATCH_BASS_SLP1       82
#define BM_PATCH_BASS_SLP2       83
#define BM_PATCH_BASS_SYN1       84
#define BM_PATCH_BASS_SYN1_WA    85
#define BM_PATCH_BASS_SYN1_RE    86
#define BM_PATCH_BASS_SYN1_CL    87
#define BM_PATCH_BASS_SYN1_HA    88
#define BM_PATCH_BASS_SYN2       89
#define BM_PATCH_BASS_SYN2_AT    90
#define BM_PATCH_BASS_SYN2_RU    91
#define BM_PATCH_BASS_SYN2_AP    92
#define BM_PATCH_STRING_VILN     93
#define BM_PATCH_STRING_VILN_SA  94
#define BM_PATCH_STRING_VILA     95
#define BM_PATCH_STRING_CELL     96
#define BM_PATCH_STRING_CONT     97
#define BM_PATCH_STRING_TREM     98
#define BM_PATCH_STRING_PIZZ     99
#define BM_PATCH_STRING_HARP    100
#define BM_PATCH_STRING_HARP_YC 101
#define BM_PATCH_STRING_TIMP    102
#define BM_PATCH_ENSEM_STR1     103
#define BM_PATCH_ENSEM_STR1_SB  104
#define BM_PATCH_ENSEM_STR1_60  105
#define BM_PATCH_ENSEM_STR2     106
#define BM_PATCH_ENSEM_SYN1     107
#define BM_PATCH_ENSEM_SYN1_AL  108
#define BM_PATCH_ENSEM_SYN2     109
#define BM_PATCH_ENSEM_CHOI     110
#define BM_PATCH_ENSEM_CHOI_AL  111
#define BM_PATCH_ENSEM_VOIC     112
#define BM_PATCH_ENSEM_VOIC_HM  113
#define BM_PATCH_ENSEM_SYVO     114
#define BM_PATCH_ENSEM_SYVO_AN  115
#define BM_PATCH_ENSEM_ORHI     116
#define BM_PATCH_ENSEM_ORHI_BP  117
#define BM_PATCH_ENSEM_ORHI_6   118
#define BM_PATCH_ENSEM_ORHI_EU  119
#define BM_PATCH_BRASS_TRUM     120
#define BM_PATCH_BRASS_TRUM_DS  121
#define BM_PATCH_BRASS_TROM     122
#define BM_PATCH_BRASS_TROM_AL  123
#define BM_PATCH_BRASS_TROM_BR  124
#define BM_PATCH_BRASS_TUBA     125
#define BM_PATCH_BRASS_MUTR     126
#define BM_PATCH_BRASS_MUTR_AL  127
#define BM_PATCH_BRASS_FRHO     128
#define BM_PATCH_BRASS_FRHO_WA  129
#define BM_PATCH_BRASS_BRSE     130
#define BM_PATCH_BRASS_BRSE_OM  131
#define BM_PATCH_BRASS_SBR1     132
#define BM_PATCH_BRASS_SBR1_AL  133
#define BM_PATCH_BRASS_SBR1_AN  134
#define BM_PATCH_BRASS_SBR1_JU  135
#define BM_PATCH_BRASS_SBR2     136
#define BM_PATCH_BRASS_SBR2_AL  137
#define BM_PATCH_BRASS_SBR2_AN  138
#define BM_PATCH_REED_SOSA      139
#define BM_PATCH_REED_ALSA      140
#define BM_PATCH_REED_TESA      141
#define BM_PATCH_REED_BASA      142
#define BM_PATCH_REED_OBOE      143
#define BM_PATCH_REED_ENHO      144
#define BM_PATCH_REED_BASS      145
#define BM_PATCH_REED_CLAR      146
#define BM_PATCH_PIPE_PICC      147
#define BM_PATCH_PIPE_FLUT      148
#define BM_PATCH_PIPE_RECO      149
#define BM_PATCH_PIPE_PAFL      150
#define BM_PATCH_PIPE_BLBO      151
#define BM_PATCH_PIPE_SHAK      152
#define BM_PATCH_PIPE_WHIS      153
#define BM_PATCH_PIPE_OCAR      154
#define BM_PATCH_LEAD_OSC1      155
#define BM_PATCH_LEAD_OSC1_SQ   156
#define BM_PATCH_LEAD_OSC1_SI   157
#define BM_PATCH_LEAD_OSC2      158
#define BM_PATCH_LEAD_OSC2_SA   159
#define BM_PATCH_LEAD_OSC2_SP   160
#define BM_PATCH_LEAD_OSC2_DS   161
#define BM_PATCH_LEAD_OSC2_AN   162
#define BM_PATCH_LEAD_CALL      163
#define BM_PATCH_LEAD_CHIF      164
#define BM_PATCH_LEAD_CHAR      165
#define BM_PATCH_LEAD_CHAR_WL   166
#define BM_PATCH_LEAD_VOIC      167
#define BM_PATCH_LEAD_FIFT      168
#define BM_PATCH_LEAD_BALE      169
#define BM_PATCH_LEAD_BALE_SW   170
#define BM_PATCH_PAD_NEAG       171
#define BM_PATCH_PAD_WARM       172
#define BM_PATCH_PAD_WARM_SI    173
#define BM_PATCH_PAD_POLY       174
#define BM_PATCH_PAD_CHOI       175
#define BM_PATCH_PAD_CHOI_IT    176
#define BM_PATCH_PAD_BOWE       177
#define BM_PATCH_PAD_META       178
#define BM_PATCH_PAD_HALO       179
#define BM_PATCH_PAD_SWEE       180
#define BM_PATCH_SFX1_RAIN      181
#define BM_PATCH_SFX1_SOTR      182
#define BM_PATCH_SFX1_CRYS      183
#define BM_PATCH_SFX1_CRYS_MA   184
#define BM_PATCH_SFX1_ATMO      185
#define BM_PATCH_SFX1_BRIG      186
#define BM_PATCH_SFX1_GOBL      187
#define BM_PATCH_SFX1_ECHO      188
#define BM_PATCH_SFX1_ECHO_BE   189
#define BM_PATCH_SFX1_ECHO_PA   190
#define BM_PATCH_SFX1_SCFI      191
#define BM_PATCH_ETHNIC_SITA    192
#define BM_PATCH_ETHNIC_SITA_BE 193
#define BM_PATCH_ETHNIC_BANJ    194
#define BM_PATCH_ETHNIC_SHAM    195
#define BM_PATCH_ETHNIC_KOTO    196
#define BM_PATCH_ETHNIC_KOTO_TA 197
#define BM_PATCH_ETHNIC_KALI    198
#define BM_PATCH_ETHNIC_BAPI    199
#define BM_PATCH_ETHNIC_FIDD    200
#define BM_PATCH_ETHNIC_SHAN    201
#define BM_PATCH_PERC_TIBE      202
#define BM_PATCH_PERC_AGOG      203
#define BM_PATCH_PERC_STDR      204
#define BM_PATCH_PERC_WOOD      205
#define BM_PATCH_PERC_WOOD_CA   206
#define BM_PATCH_PERC_TADR      207
#define BM_PATCH_PERC_TADR_CB   208
#define BM_PATCH_PERC_METO      209
#define BM_PATCH_PERC_METO_PO   210
#define BM_PATCH_PERC_SYDR      211
#define BM_PATCH_PERC_SYDR_RB   212
#define BM_PATCH_PERC_SYDR_EL   213
#define BM_PATCH_PERC_RECY      214
#define BM_PATCH_SFX2_G0_GUFR   215
#define BM_PATCH_SFX2_G0_GUCU   216
#define BM_PATCH_SFX2_G0_STSL   217
#define BM_PATCH_SFX2_G1_BRNO   218
#define BM_PATCH_SFX2_G1_FLKC   219
#define BM_PATCH_SFX2_G2_SEAS   220
#define BM_PATCH_SFX2_G2_RAIN   221
#define BM_PATCH_SFX2_G2_THUN   222
#define BM_PATCH_SFX2_G2_WIND   223
#define BM_PATCH_SFX2_G2_STRE   224
#define BM_PATCH_SFX2_G2_BUBB   225
#define BM_PATCH_SFX2_G3_BTW1   226
#define BM_PATCH_SFX2_G3_DOG    227
#define BM_PATCH_SFX2_G3_HOGA   228
#define BM_PATCH_SFX2_G3_BTW2   229
#define BM_PATCH_SFX2_G4_TEL1   230
#define BM_PATCH_SFX2_G4_TEL2   231
#define BM_PATCH_SFX2_G4_DOCR   232
#define BM_PATCH_SFX2_G4_DOOR   233
#define BM_PATCH_SFX2_G4_SCRA   234
#define BM_PATCH_SFX2_G4_WICH   235
#define BM_PATCH_SFX2_G5_HELI   236
#define BM_PATCH_SFX2_G5_CAEN   237
#define BM_PATCH_SFX2_G5_CAST   238
#define BM_PATCH_SFX2_G5_CAPA   239
#define BM_PATCH_SFX2_G5_CACR   240
#define BM_PATCH_SFX2_G5_SIRE   241
#define BM_PATCH_SFX2_G5_TRAI   242
#define BM_PATCH_SFX2_G5_JETP   243
#define BM_PATCH_SFX2_G5_STAR   244
#define BM_PATCH_SFX2_G5_BUNO   245
#define BM_PATCH_SFX2_G6_APPL   246
#define BM_PATCH_SFX2_G6_LAUG   247
#define BM_PATCH_SFX2_G6_SCRE   248
#define BM_PATCH_SFX2_G6_PUNC   249
#define BM_PATCH_SFX2_G6_HEBE   250
#define BM_PATCH_SFX2_G6_FOOT   251
#define BM_PATCH_SFX2_G7_GUSH   252
#define BM_PATCH_SFX2_G7_MAGU   253
#define BM_PATCH_SFX2_G7_LAGU   254
#define BM_PATCH_SFX2_G7_EXPL   255
#define BM_PATCH_PERSND_STAN    256
#define BM_PATCH_PERSND_ROOM    257
#define BM_PATCH_PERSND_POWE    258
#define BM_PATCH_PERSND_ELEC    259
#define BM_PATCH_PERSND_ANLG    260
#define BM_PATCH_PERSND_JAZZ    261
#define BM_PATCH_PERSND_BRUS    262
#define BM_PATCH_PERSND_ORCH    263
#define BM_PATCH_PERSND_SNFX    264

typedef struct {
	bm_ev_type type;
	union {
		uint16_t reset;       // unsigned 16-bit (0 to 65535), divisor, ticks per quarter-note
		uint32_t tempo;       // unsigned 24-bit (0 to 16777215), microseconds per quarter-note
		uint16_t mastvol;     // unsigned 14-bit (0 to 16383)
		int16_t mastpan;      // signed 14-bit (-8192 [left] to 8191 [right])
		struct {
			uint8_t channel;  // unsigned 4-bit (0 to 15)
			uint8_t note;     // unsigned 7-bit (0 to 127)
			uint8_t velocity; // unsigned 7-bit (0 to 127)
		} noteon;
		struct {
			uint8_t channel;  // unsigned 4-bit (0 to 15)
			uint8_t note;     // unsigned 7-bit (0 to 127)
		} noteoff;
		struct {
			uint8_t channel;  // unsigned 4-bit (0 to 15)
			uint8_t pedal;    // 0 to 5, see BM_PEDAL_*
		} pedalon;
		struct {
			uint8_t channel;  // unsignd 4-bit (0 to 15)
			uint8_t pedal;    // 0 to 5, see BM_PEDAL_*
		} pedaloff;
		struct {
			uint8_t channel;  // unsigned 4-bit (0 to 15)
			uint16_t vol;     // unsigned 14-bit (0 to 16383)
		} chanvol;
		struct {
			uint8_t channel;  // unsigned 4-bit (0 to 15)
			int16_t pan;      // signed 14-bit (-8192 [left] to 8191 [right])
		} chanpan;
		struct {
			uint8_t channel;  // unsigned 4-bit (0 to 15)
			uint16_t patch;   // 0 to 264, see BM_PATCH_*
		} patch;
		struct {
			uint8_t channel;  // unsigned 4-bit (0 to 15)
			int16_t bend;     // signed 14-bit (-8192 [-2 semitones] to 8191 [+2 semitones])
		} bend;
		struct {
			uint8_t channel;  // unsigned 4-bit (0 to 15)
			uint16_t mod;     // unsigned 14-bit (0 to 16383)
		} mod;
	} u;
} bm_ev_st;

typedef struct {
	uint16_t divisor;         // set via BM_EV_RESET
	uint32_t tempo;           // set via BM_EV_TEMPO
	uint16_t mastvol;         // set via BM_EV_MASTVOL
	uint16_t mastpan;         // set via BM_EV_MASTPAN
	struct {
		uint16_t vol;         // set via BM_EV_CHANVOL
		int16_t pan;          // set via BM_EV_CHANPAN
		uint16_t patch;       // set via BM_EV_PATCH
		int16_t bend;         // set via BM_EV_BEND
		uint16_t mod;         // set via BM_EV_MOD
		bool pedals[6];       // set via BM_EV_PEDALON/PEDALOFF
		struct {
			bool down;        // set via BM_EV_NOTEON/NOTEOFF
			uint8_t velocity; // set via BM_EV_NOTEON/NOTEOFF
		} notes[128];
	} channels[16];
} bm_state_st, *bm_state;

typedef struct {
	// this should be considered private, but it is exposed here to allow for static allocation
	uint32_t bank;
	uint16_t vol;
	uint16_t pan;
} bm_device_ctrl_st;

typedef struct {
	// this should be considered private, but it is exposed here to allow for static allocation
	bm_device_ctrl_st ctrls[16];
	int running_status;
} bm_device_st;

typedef struct {
	int delta;   // number of ticks from previous event
	bm_ev_st ev; // the new event
} bm_delta_ev_st;

typedef void (*bm_event_f)(bm_delta_ev_st event, void *user);
typedef void (*bm_warn_f)(const char *msg, void *user);
typedef size_t (*bm_dump_f)(const void *restrict ptr, size_t size, size_t nitems,
	void *restrict dumpuser);

const char *bm_patchstr(uint16_t patch);
void bm_init(bm_state state);
void bm_update(bm_state state, bm_ev_st *events, int events_size);
void bm_deviceinit(bm_device_st *device);
int  bm_devicebytes(bm_device_st *device, const uint8_t *data, int size, bm_ev_st *events_out,
	int max_events_size, bm_warn_f f_warn, void *user);
void bm_readmidi(const uint8_t *data, int size, bm_event_f f_event, bm_warn_f f_warn,
	void *user);
void bm_writemidi(bm_delta_ev_st *events, int size, bm_dump_f f_dump, void *user);

// calculates the number of samples that `ticks` represents, using the state's divisor and tempo,
// along with the samples per second
static inline int bm_samples(uint16_t divisor, uint32_t tempo, int sample_rate, int ticks){
	// divisor is ticks per quarter-note
	// tempo is microseconds per quarter-note
	// sample rate is samples per second
	// and ticks is... ticks
	// samples = ticks * quarternotes/tick * microsecs/quarternote * secs/microsec * samples/sec
	return (int)(((uint64_t)ticks * (uint64_t)tempo * (uint64_t)sample_rate) /
		(UINT64_C(1000000) * (uint64_t)divisor));
}

#endif // BASICMIDI__H
