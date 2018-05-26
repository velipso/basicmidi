MIDI File Format
================

This document is my attempt at simplifying and clairifying the MIDI file format.

I've used two primary sources of information:

1. [Official MIDI specifications](https://midi.org)
2. Real-world MIDI files

I also searched the Internet for random things I found in the files, to hopefully dig up some
additional context and information -- specifically, there are points where organizations can extend
MIDI with their own codes, so decoding certain messages requires (hopefully) finding information
from the appropriate manufacturer.

My intention is to document what a MIDI file *should* look like (for encoders), but also what
real-world data looks like (for lenient decoders).

Real-World Data
---------------

Thankfully, there exists a large body of real-world MIDI files to inspect:

[The Geocities MIDI Collection (2009)](https://archive.org/details/archiveteam-geocities-midi-collection-2009)

This archive has 48,197 MIDI files, totalling 1.3 GB of data.

The data also happens to be pretty damn dirty too -- which helps us in figuring out good strategies
for recovering useful information.

I've also downloaded the entire [VGMusic.com](http://vgmusic.com/) website, which provided another
30,687 MIDI files, 886 MB of data.

MIDI Overview
-------------

MIDI is a protocol for transmitting note and controller information across a serial connection.  A
good mental model is to think of MIDI as the data that comes off a music keyboard *before* being
synthesized into sound.  For example, there is a Note On event, which indicates that a key has been
pressed, and a Note Off event when it's released.

The MIDI protocol is designed to be used in real-time -- as the musician is generating events with
the keyboard, the data is being transmitted over the wire, which a synthesizer can react to in
real-time by outputting sound.

In order to store a stream of MIDI events in a file, events are timestamped.  This allows a
program to "play back" the stream of events at the correct time.

The bulk of the complexity is understanding the events themselves.  The MIDI protocol has maintained
backwards compatibility, which means it's easy to see the raw bytes that make up a event, but
understanding how to unpack and interpret each event is complicated.

General MIDI 2 is an additional layer of requirements on top of the basic MIDI protocol that further
specifies the meaning behind events, and systems/devices claiming to support GM 2 are required to
support a minimum feature set.

Chunks
------

MIDI files consist of a stream of chunks.  Each chunk has the format:

| Name       | Size                 | Description                                                 |
|------------|----------------------|-------------------------------------------------------------|
| Chunk Type | 4 bytes              | Format of the chunk, typically either `'MThd'` or `'MTrk'`  |
| Data Size  | 4 bytes (big endian) | Length of the data (not including Chunk Type of Data Size)  |
| Data       | Specified by Data Size | Raw data for the chunk, interpreted based on Chunk Type   |

A MIDI file start with a `'MThd'` chunk, followed by one or more `'MTrk'` chunks.  I've also
seen `'XFIH'` and `'XFKM'` chunks, which after searching online, shows they are extensions by
Yamaha for karaoke.

Chunks *should* be immediately one after another, and most files follow that rule -- however, a few
files have junk data between chunks (for example, a string of repeated bytes like `00 00 00 ...`),
or data that *looks* useful, but I can't figure out how to decode it.  The best strategy is probably
to start searching 7 bytes backwards (in case the previous track reported too large a Data Size),
and search forward looking for known Chunk Types until the end of the file.

The chunk stream starts with `'MThd'`, and *should* only contain `'MThd'` once.  In practice, all
MIDI files I've seen start with `'MThd'`, but some also have `'MThd'` chunks in the middle of the
file.  I suspect this should be treated like two MIDI files have been concatenated together into
one.

The largest Data Size I've seen is 362,528 bytes (`dou_01.mid`), which is 45 minutes of dense piano.
A good sanity check might be to make sure the Data Size isn't larger than, say, 1,000,000 bytes,
otherwise you're probably reading bad data.

Header Chunk `'MThd'`
---------------------

| Name         | Size                 | Description                        |
|--------------|----------------------|------------------------------------|
| Chunk Type   | 4 bytes              | `'MThd'` (`4D 54 68 64`)           |
| Data Size    | 4 bytes (big endian) | 6 (`00 00 00 06`)                  |
| Format       | 2 bytes (big endian) | `0`, `1`, or `2`, described below  |
| Track Chunks | 2 bytes (big endian) | Number of track chunks in the file |
| Division     | 2 bytes (big endian) | Timestamp format, described below  |

### Format

The Format can either be `00 00`, `00 01`, or `00 02`, each with the following meanings:

| Format  | Description                                                                           |
|---------|---------------------------------------------------------------------------------------|
| `00 00` | The file contains a single multi-channel track (so Track Chunks should be `00 01`)    |
| `00 01` | The file contains one or more tracks that should be played simultaneously             |
| `00 02` | The file contains one or more tracks that are independant of each other               |

### Track Chunks

The Track Chunks should be a count of `'MTrk'` chunks in the file.  In practice, this number is
sometimes wrong.  A decoder should probably just ignore it, because the `'MTrk'` chunks can be
counted directly.

When multiple `'MThd'` chunks are in a file (which is against the specification), sometimes the
Track Count represents the tracks up until that point, and sometimes they represent the total number
of track chunks in the entire file.  And sometimes it's just completely wrong.  Yet another reason
to ignore the field.

### Division

According to the specification, the division can come in two formats:

1. If bit 15 is cleared, then bits 14-0 represent the ticks per quarter-note
2. If bit 15 is set, then bits 14-8 represent the negative SMPTE format, and bits 7-0 represent the
   ticks per frame

In practice, no files have bit 15 set (other than `ffmqbatl.mid` which has it set due to a corrupted
header).  If someone can find me some test MIDI files that use SMPTE timing, then I would love to
look into it.  Otherwise, I will just reject files that have bit 15 set.

Figuring out the timing of the events is, unfortunately, not straight forward.  There is an entire
section on calculating timing below.

Track Chunk `'MTrk'`
--------------------

Track chunks consist of a stream of timestamped events.

| Name        | Size                 | Description                                                |
|-------------|----------------------|------------------------------------------------------------|
| Chunk Type  | 4 bytes              | `'MTrk'` (`4D 54 72 6B`)                                   |
| Data Size   | 4 bytes (big endian) | Length of the data (not including Chunk Type of Data Size) |
| MTrk Events | Data Size            | One or more MTrk events                                    |

A single MTrk event consists of a delta timestamp and an event:

| Name                 | Size         | Description                                               |
|----------------------|--------------|-----------------------------------------------------------|
| Delta Timestamp `<DT>` | Variable Int | Number of ticks this event occurs relative to previous  |
| Event                | Varies according to event | Channel Message, SysEx Event, or Meta Event  |

### Variable Int Quantities

Some MTrk events contain Variable Int quantities, like the Delta Timestamp.  This is a 32-bit
integer with a maximum value of `0FFFFFFF` (*not* `FFFFFFFF`).  It is encoded 7 bits per byte,
most significant bits first, where the most significant bit of each encoded byte is reserved to
indicate whether more bytes need to be read.

The encoding ranges from 1 to 4 bytes, with the following binary decodings:

| Encoding                              | Decoding                              |
|---------------------------------------|---------------------------------------|
| `0aaaaaaa`                            | `00000000 00000000 00000000 0aaaaaaa` |
| `1aaaaaaa 0bbbbbbb`                   | `00000000 00000000 00aaaaaa abbbbbbb` |
| `1aaaaaaa 1bbbbbbb 0ccccccc`          | `00000000 000aaaaa aabbbbbb bccccccc` |
| `1aaaaaaa 1bbbbbbb 1ccccccc 0ddddddd` | `0000aaaa aaabbbbb bbcccccc cddddddd` |

Examples:

| Encoding      | Decoding   |
|---------------|------------|
| `00`          | `00000000` |
| `67`          | `00000067` |
| `7F`          | `0000007F` |
| `81 00`       | `00000080` |
| `C6 45`       | `00002345` |
| `FF 7F`       | `00003FFF` |
| `81 80 00`    | `00004000` |
| `C8 E8 56`    | `00123456` |
| `FF FF 7F`    | `001FFFFF` |
| `81 80 80 00` | `00200000` |
| `C4 EA F9 5E` | `089ABCDE` |
| `FF FF FF 7F` | `0FFFFFFF` |

### Channel Messages

Channel Messages start with a single byte in the range `80` to `EF`.  The lower 4 bits represent
the channel `n` (0-15 are usually displayed as 1-16), and the higher 4 bits represent the message
type.  For example, `94` is a Note On message for channel 5.

| Type | Parameter 1             | Parameter 2                     | Description               |
|------|-------------------------|---------------------------------|---------------------------|
| `8n` | Note (`00` to `7F`)     | Release Velocity (`00` to `7F`) | Note Off                  |
| `9n` | Note (`00` to `7F`)     | Hit Velocity (`00` to `7F`)     | Note On                   |
| `An` | Note (`00` to `7F`)     | Pressure (`00` to `7F`)         | Note Pressure             |
| `Bn` | Control (`00` to `7F`)  | Value (`00` to `7F`)            | Control Change            |
| `Cn` | Patch (`00` to `7F`)    | N/A                             | Program Change            |
| `Dn` | Pressure (`00` to `7F`) | N/A                             | Channel Pressure          |
| `En` | Bend Change LSB (`00` to `7F`) | Bend Change MSB (`00` to `7F`) | Pitch Bend          |

#### Running Status

Running status is based on the fact that it is very common for a string of consecutive Channel
Messages to be of the same type.  For example, when a chord is played on a keyboard, ten successive
Note On messages may be generated.

If a Channel Message is processed (`8n`-`En`), then the message type should be saved.  If a future
sequence is missing a type (detected if the first byte is less than `80`), then it should be
interpretted as the same message type as the previous message.

```
<DT> 94 45 40  # Note On, note 45, velocity 40
<DT>    49 40  # Note On, note 49, velocity 40
<DT>    4D 40  # Note On, note 4D, velocity 40
<DT> 84 45 40  # Note Off, note 45, release 40
<DT>    49 40  # Note Off, note 49, release 40
<DT>    4D 40  # Note Off, note 4D, release 40
```

#### Notes

Converting bytes 0-127 to notes is based on the fact that note 69 (`45` hex) is A440.  Oddly, many
websites report the wrong octave for a given MIDI note value.  The table below assigns octaves
based on the fact that Middle C should be Octave 4.

Default tuning will be: Frequency = 440 &times; 2<sup>(Note - 69) / 12</sup>

| Octave |  C   |  C#  |  D   |  D#  |  E   |  F   |  F#  |  G   |  G#  |  A   |  A#  |  B   |
|:------:|:----:|:----:|:----:|:----:|:----:|:----:|:----:|:----:|:----:|:----:|:----:|:----:|
|  -1    | `00` | `01` | `02` | `03` | `04` | `05` | `06` | `07` | `08` | `09` | `0A` | `0B` |
|   0    | `0C` | `0D` | `0E` | `0F` | `10` | `11` | `12` | `13` | `14` | `15` | `16` | `17` |
|   1    | `18` | `19` | `1A` | `1B` | `1C` | `1D` | `1E` | `1F` | `20` | `21` | `22` | `23` |
|   2    | `24` | `25` | `26` | `27` | `28` | `29` | `2A` | `2B` | `2C` | `2D` | `2E` | `2F` |
|   3    | `30` | `31` | `32` | `33` | `34` | `35` | `36` | `37` | `38` | `39` | `3A` | `3B` |
|   4    | `3C` | `3D` | `3E` | `3F` | `40` | `41` | `42` | `43` | `44` | `45` | `46` | `47` |
|   5    | `48` | `49` | `4A` | `4B` | `4C` | `4D` | `4E` | `4F` | `50` | `51` | `52` | `53` |
|   6    | `54` | `55` | `56` | `57` | `58` | `59` | `5A` | `5B` | `5C` | `5D` | `5E` | `5F` |
|   7    | `60` | `61` | `62` | `63` | `64` | `65` | `66` | `67` | `68` | `69` | `6A` | `6B` |
|   8    | `6C` | `6D` | `6E` | `6F` | `70` | `71` | `72` | `73` | `74` | `75` | `76` | `77` |
|   9    | `78` | `79` | `7A` | `7B` | `7C` | `7D` | `7E` | `7F` | N/A  | N/A  | N/A  | N/A  |

### Control Change Message `Bn`

Different functions have been crammed into Control Change messages, so they are a bit messy to deal
with.  Some of these functions require more data than what fits in a single Control Change message,
so they span multiple Control Change messages.

Generally speaking, there are four different functions:

1. Low-resolution Controller (1 message)
2. High-resolution Controller (spans 1 or 2 messages)
3. Registered/Non-Registered Parameter (spans 3+ messages)
4. Channel Mode (1 message)

#### Low-resolution Controller

```
<DT> Bn NN VV
```

Low-resolution Controllers are the most simple.  The `NN` value ranges from `40` to `5F`, and the
7 bit value inside `VV` is the new value for that controller.  For boolean controllers, values
`00`-`3F` are considered "off", and `40`-`7F` are considered "on".  If transmitting, use `00` for
off and `7F` for on.

| `NN` | Control                                        |
|------|------------------------------------------------|
| `40` | Damper/Sustain Pedal (Hold 1) on/off           |
| `41` | Portamento on/off                              |
| `42` | Sostenuto on/off                               |
| `43` | Soft Pedal on/off                              |
| `44` | Legato Footswitch                              |
| `45` | Hold 2                                         |
| `46` | Sound Controller 1 (Sound Variation)           |
| `47` | Sound Controller 2 (Timber/Harmonic Intensity) |
| `48` | Sound Controller 3 (Release Time)              |
| `49` | Sound Controller 4 (Attack Time)               |
| `4A` | Sound Controller 5 (Brightness)                |
| `4B` | Sound Controller 6 (Decay Time)                |
| `4C` | Sound Controller 7 (Vibrato Rate)              |
| `4D` | Sound Controller 8 (Vibrato Depth)             |
| `4E` | Sound Controller 9 (Vibrato Delay)             |
| `4F` | Sound Controller 10                            |
| `50` | General Purpose 5                              |
| `51` | General Purpose 6                              |
| `52` | General Purpose 7                              |
| `53` | General Purpose 8                              |
| `54` | Portamento Control                             |
| `55` | Undefined                                      |
| `56` | Undefined                                      |
| `57` | Undefined                                      |
| `58` | High Resolution Velocity LSB (see CA-031)      |
| `59` | Undefined                                      |
| `5A` | Undefined                                      |
| `5B` | Effect 1 Depth (Reverb Send Level)             |
| `5C` | Effect 2 Depth (Tremolo Depth)                 |
| `5D` | Effect 3 Depth (Chorus Send Level)             |
| `5E` | Effect 4 Depth (Celeste Detune Depth)          |
| `5F` | Effect 5 Depth (Phaser Depth)                  |

#### High-resolution Controller

```
<DT> Bn NN MM
<DT> Bn NN LL
```

There are 32 High-resolution Controllers.  These controllers have a 14 bit value, which is split
between the 7 most significant bits (MSB `MM`), and 7 least significant bits (LSB `LL`).

When the MSB is set, the LSB should be cleared to zero.  When the LSB is set, it should overwrite
any previous LSB.  This allows a stream of LSB messages to modify the value a little bit without
needing MSB messages (with the sole exception of the Bank Select control).

Setting the MSB/LSB is determined via the value of `NN`:

| MSB `NN` | LSB `NN` | Control                           |
|----------|----------|-----------------------------------|
| `00`     | `20`     | Bank Select (LSB always required) |
| `01`     | `21`     | Modulation Wheel or Lever         |
| `02`     | `22`     | Breath Controller                 |
| `03`     | `23`     | Undefined                         |
| `04`     | `24`     | Foot Controller                   |
| `05`     | `25`     | Portamento Time                   |
| `06`     | `26`     | Data Entry (RPNs/NRPNs)           |
| `07`     | `27`     | Channel Volume                    |
| `08`     | `28`     | Balance                           |
| `09`     | `29`     | Undefined                         |
| `0A`     | `2A`     | Pan                               |
| `0B`     | `2B`     | Expression                        |
| `0C`     | `2C`     | Effect Control 1                  |
| `0D`     | `2D`     | Effect Control 2                  |
| `0E`     | `2E`     | Undefined                         |
| `0F`     | `2F`     | Undefined                         |
| `10`     | `30`     | General Purpose 1                 |
| `11`     | `31`     | General Purpose 2                 |
| `12`     | `32`     | General Purpose 3                 |
| `13`     | `33`     | General Purpose 4                 |
| `14`     | `34`     | Undefined                         |
| `15`     | `35`     | Undefined                         |
| `16`     | `36`     | Undefined                         |
| `17`     | `37`     | Undefined                         |
| `18`     | `38`     | Undefined                         |
| `19`     | `39`     | Undefined                         |
| `1A`     | `3A`     | Undefined                         |
| `1B`     | `3B`     | Undefined                         |
| `1C`     | `3C`     | Undefined                         |
| `1D`     | `3D`     | Undefined                         |
| `1E`     | `3E`     | Undefined                         |
| `1F`     | `3F`     | Undefined                         |

So, for example, setting the General Purpose 2 value to `12 34` on channel 8 (`7`) would mean the
sequence:

```
<DT> B7 11 12  # Set MSB of General Purpose 2 on channel 8 to 12H
<DT> B7 31 34  # Set LSB of General Purpose 2 on channel 8 to 34H
```

Note that the value of `12 34` is actually the number 2356, *not* 4660, because 7 bits per byte are
used.

Balance (`08`/`28`) and Pan (`0A`/`2A`) values are defined in the specification that `00 00` is
hard-left and `7F 7F` is hard-right.

The default value for Channel Volume should be `64 00` (i.e. 100 / 127).

TODO: Volume + Expression formula (see GM2 docs)

TODO: Panning formula (see GM2 docs)

TODO: Bank Select of `79 XX` corresponds to the GM1 Sound Set

TODO: Bank Select of `78 XX` corresponds to the GM1 Drum Set

TODO: All channels default to `79 00` except Channel 10, which defaults to `78 00`.

#### Registered/Non-Registered Parameter

Registered Parameter Numbers (RPNs) and Non-Registered Parameter Numbers (NRPNs) expand the space of
possible control values that can be changed.  RPNs are reserved for official parameters (table
below), and NRPNs are reserved for private use.

Each channel has a currently selected RPN/NRPN, which defaults to `7F 7F` (indicating no parameter
is selected).

To change an RPN/NRPN value, first select the parameter using:

```
<DT> Bn 65 MM  # Select RPN MSB
<DT> Bn 64 LL  # Select RPN LSB
```

or:

```
<DT> Bn 63 MM  # Select NRPN MSB
<DT> Bn 62 LL  # Select NRPN LSB
```

(As far as I can tell, the order of the messages selecting the parameter shouldn't matter)

Once a parameter is selected, it can be set using the Data Entry messages under the High-resolution
Controller (`06`/`26`).

The value can also be incremented or decremented using messages `Bn 60 XX` (increment), or
`Bn 61 XX` (decrement).  The data values (`XX`) are ignored.  The value is
incremented/decremented differently, based on which RPN selected (explained below).

Once the RPN/NRPN is finished being modified, it's recommended to select the `7F 7F` parameter
again, indicating no parameter is selected.

| MSB `MM` | LSB `LL` | Control                                         | Inc/Dec              |
|----------|----------|-------------------------------------------------|----------------------|
| `00`     | `00`     | Pitch Bend Range (MSB = semitones, LSB = cents) | LSB, wrapping at 100 |
| `00`     | `01`     | Fine Tuning                                     | LSB                  |
| `00`     | `02`     | Coarse Tuning                                   | MSB                  |
| `00`     | `03`     | Tuning Program Select                           | MSB                  |
| `00`     | `04`     | Tuning Bank Select                              | MSB                  |
| `00`     | `05`     | Modulation Depth Range                          | LSB (?)              |
| `3D`     | `00`     | Azimuth Angle                                   | LSB (?)              |
| `3D`     | `01`     | Elevation Angle                                 | LSB (?)              |
| `3D`     | `02`     | Gain                                            | LSB (?)              |
| `3D`     | `03`     | Distance Ratio                                  | LSB (?)              |
| `3D`     | `04`     | Maximum Distance                                | LSB (?)              |
| `3D`     | `05`     | Gain at Maximum Distance                        | LSB (?)              |
| `3D`     | `06`     | Reference Distance Ratio                        | LSB (?)              |
| `3D`     | `07`     | Pan Spread Angle                                | LSB (?)              |
| `3D`     | `08`     | Roll Angle                                      | LSB (?)              |
| `7F`     | `7F`     | Deselect RPN                                    | LSB (?)              |

Note that increment and decrement commands affect RPNs differently, depending on which RPN is
selected.  When the LSB over/underflows, the MSB should be affected accordingly.  Pitch Bend Range
in particular is interesting because the LSB represents cents, which means it should wrap at 100
(not 127).  See RP-018.

The default value for Pitch Bend Range is `02 00`, which is 2 semitones (i.e., a Pitch Bend message
should affect the pitch at most &plusmn;2 semitones).

#### Channel Mode

```
<DT> Bn NN VV
```

Lastly, there are a few Controllers that have special meaning, with `NN` ranging from `78`-`7F`.

| `NN` | `VV` | Action                                                           |
|------|------|------------------------------------------------------------------|
| `78` | `00` | All Sound Off                                                    |
| `79` | `00` | Reset All Controllers                                            |
| `7A` | `VV` | Local Control (`VV` = `00` off, `7F` on)                         |
| `7B` | `00` | All Notes Off                                                    |
| `7C` | `00` | Omni Off                                                         |
| `7D` | `00` | Omni On                                                          |
| `7E` | `VV` | Mono On (Poly Off) (`VV` = number of channels, `00` for all)     |
| `7F` | `00` | Poly On (Mono Off)                                               |

#### Undefined Controllers

If you've been paying attention, there are a range of controllers that aren't allocated, `66`-`77`.
These are reserved as single byte controllers.

### SysEx Event

System Exclusive (SysEx) Events are used for transfering files/samples, timing information, and
tuning.

| Name         | Size         | Description                                                      |
|--------------|--------------|------------------------------------------------------------------|
| `F0` or `F7` | 1 byte       | Byte indicating a SysEx Event                                    |
| Data Size    | Variable Int | Length of the data for this event                                |
| Data         | Data Size    | Raw data associated with SysEx Event                             |

SysEx data can be broken into packets and sent as multiple SysEx Events, requiring the parser to
piece together the parts before processing the entire event.  According to the MIDI specification,
some systems will send packeted data at certain time intervals -- so the delay between packets could
be meaningful too.

In general though, software parsers should probably just ignore SysEx Events that it can't
recognize -- it's easy to skip over the data since the length is always known.

#### General MIDI SysEx

There is a SysEx Event for enabling General MIDI or General MIDI 2.

```
<DT> F0 05 7E TT 09 GG F7
```

Where `TT` is the device target (usually `7F` to mean all devices), and `GG` is the General MIDI
command:

| `GG` | Description       |
|------|-------------------|
| `01` | General MIDI On   |
| `02` | General MIDI Off  |
| `03` | General MIDI 2 On |

#### Master Volume SysEx

```
<DT> F0 07 7F TT 04 01 LL MM F7
```

Sets the Master Volume, where `TT` is the device target (usually `7F`), and `LL MM` is the value,
where `LL` is the LSB (`00`-`7F`) and `MM` is the MSB (`00`-`7F`).

#### Master Balance SysEx

```
<DT> F0 07 7F TT 04 02 LL MM F7
```

Sets the Master Balance, where `TT` is the device target (usually `7F`), and `LL MM` is the value,
where `LL` is the LSB (`00`-`7F`) and `MM` is the MSB (`00`-`7F`).  Hard-left is `00 00` and
hard-right is `7F 7F`.

### Meta Event

Meta Events are always at least 3 bytes in length.

The first byte is `FF` which is what identifies the event as a Meta Event.

The second byte determines the type of meta event.

The third byte determines the byte length of the remaining parameters.  This is useful because you
don't need to understand the meta event type in order to skip over the event.

Any parameters spanning multiple bytes should be interpreted as big endian.

| Byte Sequence       | Description                                                               |
|---------------------|---------------------------------------------------------------------------|
| `FF 00 02 SS SS`    | Sequence Number `SSSS`                                                    |
| `FF 01 LL text`     | Generic Text of length `LL`                                               |
| `FF 02 LL text`     | Copyright Notice of length `LL`                                           |
| `FF 03 LL text`     | Track Name of length `LL`                                                 |
| `FF 04 LL text`     | Instrument Name of length `LL`                                            |
| `FF 05 LL text`     | Lyric of length `LL`                                                      |
| `FF 06 LL text`     | Marker of length `LL` ('First Verse', 'Chrous', etc)                      |
| `FF 07 LL text`     | Cue Point of length `LL` ('curtain opens', 'character is slapped', etc)   |
| `FF 08 LL text`     | Program Name of length `LL` (see RP-019)                                  |
| `FF 09 LL text`     | Device Name of length `LL` (see RP-019)                                   |
| `FF 20 01 NN`       | Channel Prefix, select channel `NN` (0-15) for future SysEx/Meta events   |
| `FF 21 01 PP`       | MIDI Port, select output port `PP` for MIDI events                        |
| `FF 2E ?? ????`     | Track Loop Event (?)                                                      |
| `FF 2F 00`          | End of Track, required as last event in a MTrk chunk                      |
| `FF 51 03 TT TT TT` | Set Tempo, in `TTTTTT` microseconds per MIDI quarter-note                 |
| `FF 54 05 HH MM SS RR TT` | SMPTE Offset, hour, minute, second, frame, 1/100th of frame         |
| `FF 58 04 NN MM LL TT` | Time Signature, see below for details                                  |
| `FF 59 02 SS MM`    | Key Signature, see below for details                                      |
| `FF 7F LL data`     | Sequencer-Specific Meta Event, see below for details                      |

#### Key Signature Meta Event

TODO

#### Sequencer-Specific Meta Event

TODO

### Calculating Timing

The MIDI specification has a lot of ways to represent timing.

The unit of the Delta Timestamps is *ticks* passed since previous event.  But what is a *tick*?
Specifically, we need to calculate *samples per tick*, so that when we see how many ticks until the
next event, we can move forward the appropriate amount of samples.

The unit of the Division parameter in the header is *ticks per quarter-note*.

The unit of the Set Tempo Meta Event (`TTTTTT`) is *microseconds per quarter-note*.  Reminder: there
are 1,000,000 *microseconds per second*.

If Set Tempo is not set, then the default value is 120 *beats per minute*.

The Time Signature Meta Event's denominator (`MM`) represents *quarter-notes per beat* in a goofy
way:

| `MM` | Time Signature Denominator | Quarter-Notes per Beat |
|------|----------------------------|------------------------|
| `00` | 1                          | 4                      |
| `01` | 2                          | 2                      |
| `02` | 4                          | 1                      |
| `03` | 8                          | <sup>1</sup>/<sub>2</sub> |
| `04` | 16                         | <sup>1</sup>/<sub>4</sub> |
| `MM` | 2<sup>`MM`</sup>           | 2<sup>2 - `MM`</sup>   |

If Time Signature is not set, the denominator defaults to 4, which represents 1 quarter-note per
beat.

The MIDI specification also talks about another unit, the *MIDI clock*, which is simply
<sup>1</sup>/<sub>24th</sub> the length of a quarter-note.

Lastly, Sample Rate (*samples per second*) is provided by the audio output format.

So with all this information, we can calculate the *samples per tick*:

We start assuming 120 *beats per minute* and 1 *quarter-note per beat*, therefore we have 120
*quarter-notes per minute*, or 2 *quarter-notes per second*.

When we receive the Division parameter from the header (*ticks per quarter-note*) and the Sample
Rate (*samples per second*) from the audio device, we can calculate *samples per tick* via:
Sample Rate / (Division &times; 2)

If we receive a Set Tempo event (`TTTTTT` *microseconds per quarter-note*), we can recalculate
*samples per tick* to be: (Sample Rate &times; `TTTTTT`) / (1,000,000 &times; Division)

If a Time Signature Meta Event comes after a Set Tempo event, it can safely be ignored -- it will
change the size of a quarter-note, but that doesn't matter because the calculation cancels out the
quarter-notes.

However, if a Time Signature Meta Event comes *before* a Set Tempo event, then that will change the
quarter-notes per beat, which means we'll use the 120 beats per minute default tempo (2 beats per
second) to figure out the *samples per tick*: Sample Rate /
(Division &times; 2 &times; 2<sup>2 - `MM`</sup>)

One last note: there needs to be care taken when calculating timing because a MIDI file can change
the tempo or time signature during the middle of the song, which will change how ticks map to
samples.

Appendix: SysEx Manufacturer ID Numbers
========================================

Assignment of an ID number requires payment to MIDI.org every year.  According to the website, those
who stop paying have their ID's rescinded.  I'm not sure what that means from a technical
standpoint - it doesn't look like the ID numbers are reallocated to new companies.  As of 2017, the
fee ranges from $500 to $20000 per year, depending on the size of the company.

| ID         | Company                                                                            |
|------------|------------------------------------------------------------------------------------|
| `00`       | Used for extensions, below                                                         |
| `01`       | Sequential                                                                         |
| `02`       | IDP                                                                                |
| `03`       | Voyetra Turtle Beach, Inc. / Octave-Plateau                                        |
| `04`       | Moog Music                                                                         |
| `05`       | Passport Designs                                                                   |
| `06`       | Lexicon Inc.                                                                       |
| `07`       | Kurzweil / Young Chang                                                             |
| `08`       | Fender                                                                             |
| `09`       | MIDI9 / Gulbransen                                                                 |
| `0A`       | AKG Acoustics                                                                      |
| `0B`       | Voyce Music                                                                        |
| `0C`       | Waveframe Corp (Timeline)                                                          |
| `0D`       | ADA Signal Processors, Inc.                                                        |
| `0E`       | Garfield Electronics                                                               |
| `0F`       | Ensoniq                                                                            |
| `10`       | Oberheim / Gibson Labs                                                             |
| `11`       | Apple, Inc.                                                                        |
| `12`       | Grey Matter Response                                                               |
| `13`       | Digidesign Inc. (rescinded)                                                        |
| `14`       | Palmtree Instruments                                                               |
| `15`       | JLCooper Electronics                                                               |
| `16`       | Lowrey Organ Company                                                               |
| `17`       | Adams-Smith                                                                        |
| `18`       | E-mu / Ensoniq                                                                     |
| `19`       | Harmony Systems                                                                    |
| `1A`       | ART                                                                                |
| `1B`       | Baldwin                                                                            |
| `1C`       | Eventide                                                                           |
| `1D`       | Inventronics                                                                       |
| `1E`       | Key Concepts (rescinded)                                                           |
| `1F`       | Clarity                                                                            |
| `20`       | Passac                                                                             |
| `21`       | Proel Labs (SIEL)                                                                  |
| `22`       | Synthaxe (UK)                                                                      |
| `23`       | Stepp (rescinded)                                                                  |
| `24`       | Hohner                                                                             |
| `25`       | Twister                                                                            |
| `26`       | Ketron s.r.l. / Solton                                                             |
| `27`       | Jellinghaus MS                                                                     |
| `28`       | Southworth Music Systems                                                           |
| `29`       | PPG (Germany)                                                                      |
| `2A`       | JEN                                                                                |
| `2B`       | Solid State Logic Organ Systems / SSL Limited                                      |
| `2C`       | Audio Veritrieb-P. Struven                                                         |
| `2D`       | Neve (rescinded)                                                                   |
| `2E`       | Soundtracs Ltd. (rescinded)                                                        |
| `2F`       | Elka                                                                               |
| `30`       | Dynacord                                                                           |
| `31`       | Viscount International Spa (Intercontinental Electronics) (rescinded)              |
| `32`       | Drawmer (rescinded)                                                                |
| `33`       | Clavia Digital Instruments                                                         |
| `34`       | Audio Architecture                                                                 |
| `35`       | GeneralMusic Corp SpA                                                              |
| `36`       | Cheetah Marketing (rescinded)                                                      |
| `37`       | C.T.M. (rescinded)                                                                 |
| `38`       | Simmons UK (rescinded)                                                             |
| `39`       | Soundcraft Electronics                                                             |
| `3A`       | Steinberg Media Technologies AG (rescinded)                                        |
| `3B`       | Wersi Gmbh                                                                         |
| `3C`       | AVAB Niethammer AB / Avab Electronik                                               |
| `3D`       | Digigram                                                                           |
| `3E`       | Waldorf Electronics GmbH                                                           |
| `3F`       | Quasimidi                                                                          |
| `40`       | Kawai Musical Instruments MFG. Co. Ltd.                                            |
| `41`       | Roland Corporation                                                                 |
| `42`       | Korg Inc.                                                                          |
| `43`       | Yamaha Corporation                                                                 |
| `44`       | Casio Computer Co. Ltd.                                                            |
| `45`       | ?                                                                                  |
| `46`       | Kamiya Studio Co. Ltd.                                                             |
| `47`       | Akai Electric Co. Ltd.                                                             |
| `48`       | Victor Company of Japan Ltd.                                                       |
| `49`       | Mesosha                                                                            |
| `4A`       | Hoshino Gakki                                                                      |
| `4B`       | Fujitsu Elect Ltd.                                                                 |
| `4C`       | Sony Corporation                                                                   |
| `4D`       | Nisshin Onpa                                                                       |
| `4E`       | Teac Corporation                                                                   |
| `4F`       | ?                                                                                  |
| `50`       | Matsushita Electric Industrial Co. Ltd.                                            |
| `51`       | Fostex Corporation                                                                 |
| `52`       | Zoom Corporation                                                                   |
| `53`       | Midori Electronics                                                                 |
| `54`       | Matsushita Communication Industrial Co. Ltd.                                       |
| `55`       | Suzuki Musical Instruments MFG. Co. Ltd.                                           |
| `56`       | Fuji Sound Corporation Ltd.                                                        |
| `57`       | Acoustic Technical Laboratory Inc.                                                 |
| `58`       | ?                                                                                  |
| `59`       | Faith Inc.                                                                         |
| `5A`       | Internet Corporation                                                               |
| `5B`       | ?                                                                                  |
| `5C`       | Seekers Co. Ltd.                                                                   |
| `5D`       | ?                                                                                  |
| `5E`       | ?                                                                                  |
| `5F`       | SD Card Association                                                                |
| `60`       | ?                                                                                  |
| `61`       | ?                                                                                  |
| `62`       | ?                                                                                  |
| `63`       | ?                                                                                  |
| `64`       | ?                                                                                  |
| `65`       | ?                                                                                  |
| `66`       | ?                                                                                  |
| `67`       | ?                                                                                  |
| `68`       | ?                                                                                  |
| `69`       | ?                                                                                  |
| `6A`       | ?                                                                                  |
| `6B`       | ?                                                                                  |
| `6C`       | ?                                                                                  |
| `6D`       | ?                                                                                  |
| `6E`       | ?                                                                                  |
| `6F`       | ?                                                                                  |
| `70`       | ?                                                                                  |
| `71`       | ?                                                                                  |
| `72`       | ?                                                                                  |
| `73`       | ?                                                                                  |
| `74`       | ?                                                                                  |
| `75`       | ?                                                                                  |
| `76`       | ?                                                                                  |
| `77`       | ?                                                                                  |
| `78`       | ?                                                                                  |
| `79`       | ?                                                                                  |
| `7A`       | ?                                                                                  |
| `7B`       | ?                                                                                  |
| `7C`       | ?                                                                                  |
| `7D`       | ?                                                                                  |
| `7E`       | Reserved for Non-Real Time Universal SysEx Messages                                |
| `7F`       | Reserved for Real Time Universal SysEx Messages                                    |
| --         | --                                                                                 |
| `00 00 00` | ?                                                                                  |
| `00 00 01` | Time/Warner Interactive                                                            |
| `00 00 02` | Advanced Gravis Comp. Tech Ltd. (rescinded)                                        |
| `00 00 03` | Media Vision (rescinded)                                                           |
| `00 00 04` | Dornes Research Group (rescinded)                                                  |
| `00 00 05` | K-Muse (rescinded)                                                                 |
| `00 00 06` | Stypher (rescinded)                                                                |
| `00 00 07` | Digital Music Corp.                                                                |
| `00 00 08` | IOTA Systems                                                                       |
| `00 00 09` | New England Digital                                                                |
| `00 00 0A` | Artisyn                                                                            |
| `00 00 0B` | IVL Technologies Ltd.                                                              |
| `00 00 0C` | Southern Music Systems                                                             |
| `00 00 0D` | Lake Butler Sound Company                                                          |
| `00 00 0E` | Alesis Studio Electronics                                                          |
| `00 00 0F` | Sound Creation (rescinded)                                                         |
| `00 00 10` | DOD Electronics Corp.                                                              |
| `00 00 11` | Studer-Editech                                                                     |
| `00 00 12` | Sonus (rescinded)                                                                  |
| `00 00 13` | Temporal Acuity Products (rescinded)                                               |
| `00 00 14` | Perfect Fretworks                                                                  |
| `00 00 15` | KAT Inc.                                                                           |
| `00 00 16` | Opcode Systems                                                                     |
| `00 00 17` | Rane Corporation                                                                   |
| `00 00 18` | Anadi Electronique                                                                 |
| `00 00 19` | KMX                                                                                |
| `00 00 1A` | Allen & Heath Brenell                                                              |
| `00 00 1B` | Peavey Electronics                                                                 |
| `00 00 1C` | 360 Systems                                                                        |
| `00 00 1D` | Spectrum Design and Development                                                    |
| `00 00 1E` | Marquis Music                                                                      |
| `00 00 1F` | Zeta Systems                                                                       |
| `00 00 20` | Axxes (Brian Parsonett)                                                            |
| `00 00 21` | Orban                                                                              |
| `00 00 22` | Indian Valley Mfg. (rescinded)                                                     |
| `00 00 23` | Triton (rescinded)                                                                 |
| `00 00 24` | KTI                                                                                |
| `00 00 25` | Breakaway Technologies                                                             |
| `00 00 26` | Leprecon / CAE Inc.                                                                |
| `00 00 27` | Harrison Systems Inc. (rescinded)                                                  |
| `00 00 28` | Future Lab/Mark Kuo (rescinded)                                                    |
| `00 00 29` | Rocktron Corporation                                                               |
| `00 00 2A` | PianoDisc                                                                          |
| `00 00 2B` | Cannon Research Group                                                              |
| `00 00 2C` | ?                                                                                  |
| `00 00 2D` | Rodgers Instrument LLC                                                             |
| `00 00 2E` | Blue Sky Logic                                                                     |
| `00 00 2F` | Encore Electronics                                                                 |
| `00 00 30` | Uptown                                                                             |
| `00 00 31` | Voce                                                                               |
| `00 00 32` | CTI Audio, Inc. (Musically Intel. Devs.)                                           |
| `00 00 33` | S3 Incorporated                                                                    |
| `00 00 34` | Broderbund / Red Orb                                                               |
| `00 00 35` | Allen Organ Co.                                                                    |
| `00 00 36` | ?                                                                                  |
| `00 00 37` | Music Quest (rescinded)                                                            |
| `00 00 38` | Aphex                                                                              |
| `00 00 39` | Gallien Krueger                                                                    |
| `00 00 3A` | IBM                                                                                |
| `00 00 3B` | Mark Of The Unicorn                                                                |
| `00 00 3C` | Hotz Corporation                                                                   |
| `00 00 3D` | ETA Lighting                                                                       |
| `00 00 3E` | NSI Corporation                                                                    |
| `00 00 3F` | Ad Lib, Inc.                                                                       |
| `00 00 40` | Richmond Sound Design                                                              |
| `00 00 41` | Microsoft                                                                          |
| `00 00 42` | Mindscape (Software Toolworks)                                                     |
| `00 00 43` | Russ Jones Marketing / Niche                                                       |
| `00 00 44` | Intone                                                                             |
| `00 00 45` | Advanced Remote Technologies (rescinded)                                           |
| `00 00 46` | White Instruments (rescinded)                                                      |
| `00 00 47` | GT Electronics/Groove Tubes                                                        |
| `00 00 48` | Pacific Research & Engineering (rescinded)                                         |
| `00 00 49` | Timeline Vista, Inc.                                                               |
| `00 00 4A` | Mesa Boogie Ltd.                                                                   |
| `00 00 4B` | FSLI (rescinded)                                                                   |
| `00 00 4C` | Sequoia Development Group                                                          |
| `00 00 4D` | Studio Electronics                                                                 |
| `00 00 4E` | Euphonix, Inc                                                                      |
| `00 00 4F` | InterMIDI, Inc.                                                                    |
| `00 00 50` | MIDI Solutions Inc.                                                                |
| `00 00 51` | 3DO Company                                                                        |
| `00 00 52` | Lightwave Research / High End Systems                                              |
| `00 00 53` | Micro-W Corporation                                                                |
| `00 00 54` | Spectral Synthesis, Inc.                                                           |
| `00 00 55` | Lone Wolf                                                                          |
| `00 00 56` | Studio Technologies Inc.                                                           |
| `00 00 57` | Peterson Electro-Musical Product (EMP), Inc.                                       |
| `00 00 58` | Atari Corporation                                                                  |
| `00 00 59` | Marion Systems Corporation                                                         |
| `00 00 5A` | Design Event                                                                       |
| `00 00 5B` | Winjammer Software Ltd.                                                            |
| `00 00 5C` | AT&T Bell Laboratories                                                             |
| `00 00 5D` | ?                                                                                  |
| `00 00 5E` | Symetrix                                                                           |
| `00 00 5F` | MIDI the World                                                                     |
| `00 00 60` | Spatializer / Desper Products                                                      |
| `00 00 61` | Micros 'N MIDI                                                                     |
| `00 00 62` | Accordians International                                                           |
| `00 00 63` | 3Com / EuPhonics                                                                   |
| `00 00 64` | Musonix                                                                            |
| `00 00 65` | Turtle Beach Systems (Voyetra)                                                     |
| `00 00 66` | Loud Technologies / Mackie Designs                                                 |
| `00 00 67` | Compuserve                                                                         |
| `00 00 68` | BEC Technologies                                                                   |
| `00 00 69` | QRS Music Inc                                                                      |
| `00 00 6A` | P.G. Music                                                                         |
| `00 00 6B` | Sierra Semiconductor                                                               |
| `00 00 6C` | EpiGraf Audio Visual                                                               |
| `00 00 6D` | Electronics Diversified Inc                                                        |
| `00 00 6E` | Tune 1000                                                                          |
| `00 00 6F` | Advanced Micro Devices                                                             |
| `00 00 70` | Mediamation                                                                        |
| `00 00 71` | Sabine Musical Mfg. Co. Inc.                                                       |
| `00 00 72` | Woog Labs                                                                          |
| `00 00 73` | Micropolis Corp                                                                    |
| `00 00 74` | Ta Horng Musical Inst.                                                             |
| `00 00 75` | eTek Labs / Forte Tech                                                             |
| `00 00 76` | Electrovoice                                                                       |
| `00 00 77` | Midisoft Corporation                                                               |
| `00 00 78` | Q-Sound Labs (rescinded)                                                           |
| `00 00 79` | Westrex                                                                            |
| `00 00 7A` | NVidia                                                                             |
| `00 00 7B` | ESS Technology                                                                     |
| `00 00 7C` | MediaTrix Peripherals                                                              |
| `00 00 7D` | Brooktree Corp                                                                     |
| `00 00 7E` | Otari Corp                                                                         |
| `00 00 7F` | Key Electronics, Inc.                                                              |
| `00 00 80` | Crystalake Multimedia                                                              |
| `00 00 81` | Crystal Semiconductor                                                              |
| `00 00 82` | Rockwell Semiconductor                                                             |
| --         | --                                                                                 |
| `00 01 00` | Shure Incorporated                                                                 |
| `00 01 01` | AuraSound                                                                          |
| `00 01 02` | Crystal Semiconductor                                                              |
| `00 01 03` | Conexant (Rockwell)                                                                |
| `00 01 04` | Silicon Graphics                                                                   |
| `00 01 05` | M-Audio (Midiman)                                                                  |
| `00 01 06` | PreSonus                                                                           |
| `00 01 07` | ?                                                                                  |
| `00 01 08` | Topaz Enterprises                                                                  |
| `00 01 09` | Cast Lighting                                                                      |
| `00 01 0A` | Microsoft                                                                          |
| `00 01 0B` | Sonic Foundry                                                                      |
| `00 01 0C` | Line 6 (Fast Forward) (Yamaha)                                                     |
| `00 01 0D` | Beatnik Inc                                                                        |
| `00 01 0E` | Van Koevering Company                                                              |
| `00 01 0F` | Altech Systems                                                                     |
| `00 01 10` | S & S Research                                                                     |
| `00 01 11` | VLSI Technology                                                                    |
| `00 01 12` | Chromatic Research                                                                 |
| `00 01 13` | Sapphire                                                                           |
| `00 01 14` | IDRC                                                                               |
| `00 01 15` | Justonic Tuning                                                                    |
| `00 01 16` | TorComp Research Inc.                                                              |
| `00 01 17` | Newtek Inc.                                                                        |
| `00 01 18` | Sound Sculpture                                                                    |
| `00 01 19` | Walker Technical                                                                   |
| `00 01 1A` | Digital Harmony (PAVO)                                                             |
| `00 01 1B` | InVision Interactive                                                               |
| `00 01 1C` | T-Square Design                                                                    |
| `00 01 1D` | Nemesys Music Technology                                                           |
| `00 01 1E` | DBX Professional (Harman Intl)                                                     |
| `00 01 1F` | Syndyne Corporation                                                                |
| `00 01 20` | Bitheadz                                                                           |
| `00 01 21` | Cakewalk Music Software (Gibson)                                                   |
| `00 01 22` | Analog Devices                                                                     |
| `00 01 23` | National Semiconductor                                                             |
| `00 01 24` | Boom Theory / Adinolfi Alternative Percussion                                      |
| `00 01 25` | Virtual DSP Corporation                                                            |
| `00 01 26` | Antares Systems                                                                    |
| `00 01 27` | Angel Software                                                                     |
| `00 01 28` | St Louis Music                                                                     |
| `00 01 29` | Passport Music Software LLC (Gvox) (rescinded)                                     |
| `00 01 2A` | Ashley Audio Inc.                                                                  |
| `00 01 2B` | Vari-Lite Inc.                                                                     |
| `00 01 2C` | Summit Audio Inc.                                                                  |
| `00 01 2D` | Aureal Semiconductor Inc.                                                          |
| `00 01 2E` | SeaSound LLC                                                                       |
| `00 01 2F` | U.S. Robotics                                                                      |
| `00 01 30` | Aurisis Research                                                                   |
| `00 01 31` | Nearfield Research                                                                 |
| `00 01 32` | FM7 Inc                                                                            |
| `00 01 33` | Swivel Systems                                                                     |
| `00 01 34` | Hyperactive Audio Systems                                                          |
| `00 01 35` | MidiLite (Castle Studios Productions)                                              |
| `00 01 36` | Radikal Technologies                                                               |
| `00 01 37` | Roger Linn Design (rescinded)                                                      |
| `00 01 38` | TC-Helicon Vocal Technologies (rescinded)                                          |
| `00 01 39` | Event Electronics (rescinded)                                                      |
| `00 01 3A` | Sonic Network Inc                                                                  |
| `00 01 3B` | Realtime Music Solutions (rescinded)                                               |
| `00 01 3C` | Apogee Digital (rescinded)                                                         |
| `00 01 3D` | Classical Organs, Inc. (rescinded)                                                 |
| `00 01 3E` | Microtools Inc. (rescinded)                                                        |
| `00 01 3F` | Numark Industries                                                                  |
| `00 01 40` | Frontier Design Group, LLC                                                         |
| `00 01 41` | Recordare LLC                                                                      |
| `00 01 42` | Starr Labs (rescinded)                                                             |
| `00 01 43` | Voyager Sound Inc.                                                                 |
| `00 01 44` | Manifold Labs (rescinded)                                                          |
| `00 01 45` | Aviom Inc. (rescinded)                                                             |
| `00 01 46` | Mixmeister Technology                                                              |
| `00 01 47` | Notation Software                                                                  |
| `00 01 48` | Mercurial Communications                                                           |
| `00 01 49` | Wave Arts                                                                          |
| `00 01 4A` | Logic Sequencing Devices (rescinded)                                               |
| `00 01 4B` | Axess Electronics                                                                  |
| `00 01 4C` | Muse Research                                                                      |
| `00 01 4D` | Open Labs (rescinded)                                                              |
| `00 01 4E` | Guillemot Corp (rescinded)                                                         |
| `00 01 4F` | Samson Technologies (rescinded)                                                    |
| `00 01 50` | Electronic Theatre Controls                                                        |
| `00 01 51` | Blackberry (RIM)                                                                   |
| `00 01 52` | Mobileer                                                                           |
| `00 01 53` | Synthogy                                                                           |
| `00 01 54` | Lynx Studio Technology Inc.                                                        |
| `00 01 55` | Damage Control Engineering LLC                                                     |
| `00 01 56` | Yost Engineering, Inc. (rescinded)                                                 |
| `00 01 57` | Brooks & Forsman Designs LLC / DrumLite                                            |
| `00 01 58` | Infinite Response                                                                  |
| `00 01 59` | Garritan Corp                                                                      |
| `00 01 5A` | Plogue Art et Technologie, Inc (rescinded)                                         |
| `00 01 5B` | RJM Music Technology                                                               |
| `00 01 5C` | Custom Solutions Software                                                          |
| `00 01 5D` | Sonarcana LLC / Highly Liquid                                                      |
| `00 01 5E` | Centrance (rescinded)                                                              |
| `00 01 5F` | Kesumo LLC                                                                         |
| `00 01 60` | Stanton (Gibson)                                                                   |
| `00 01 61` | Livid Instruments                                                                  |
| `00 01 62` | First Act / 745 Media                                                              |
| `00 01 63` | Pygraphics, Inc.                                                                   |
| `00 01 64` | Panadigm Innovations Ltd.                                                          |
| `00 01 65` | Avedis Zildjian Co                                                                 |
| `00 01 66` | Auvital Music Corp (rescinded)                                                     |
| `00 01 67` | You Rock Guitar / Inspired Instruments (rescinded)                                 |
| `00 01 68` | Chris Grigg Designs                                                                |
| `00 01 69` | Slate Digital LLC                                                                  |
| `00 01 6A` | Mixware  (rescinded)                                                               |
| `00 01 6B` | Social Entropy                                                                     |
| `00 01 6C` | Source Audio LLC                                                                   |
| `00 01 6D` | Ernie Ball / Music Man                                                             |
| `00 01 6E` | Fishman                                                                            |
| `00 01 6F` | Custom Audio Electronics (rescinded)                                               |
| `00 01 70` | American Audio/DJ                                                                  |
| `00 01 71` | Mega Control Systems (rescinded)                                                   |
| `00 01 72` | Kilpatrick Audio                                                                   |
| `00 01 73` | iConnectivity                                                                      |
| `00 01 74` | Fractal Audio                                                                      |
| `00 01 75` | NetLogic Microsystems                                                              |
| `00 01 76` | Music Computing                                                                    |
| `00 01 77` | Nektar Technology Inc                                                              |
| `00 01 78` | Zenph Sound Innovations                                                            |
| `00 01 79` | DJTechTools.com (rescinded)                                                        |
| `00 01 7A` | Rezonance Labs                                                                     |
| `00 01 7B` | Decibel Eleven                                                                     |
| `00 01 7C` | CNMAT (rescinded)                                                                  |
| `00 01 7D` | Media Overkill                                                                     |
| `00 01 7E` | Confusion Studios (rescinded)                                                      |
| `00 01 7F` | moForte Inc                                                                        |
| --         | --                                                                                 |
| `00 02 00` | Miselu Inc                                                                         |
| `00 02 01` | Amelia's Compass LLC                                                               |
| `00 02 02` | Zivix LLC                                                                          |
| `00 02 03` | Artiphon                                                                           |
| `00 02 04` | Synclavier Digital                                                                 |
| `00 02 05` | Light & Sound Control Devices LLC (rescinded)                                      |
| `00 02 06` | Retronyms Inc                                                                      |
| `00 02 07` | JS Technologies                                                                    |
| `00 02 08` | Quicco Sound                                                                       |
| `00 02 09` | A-Designs Audio                                                                    |
| `00 02 0A` | McCarthy Music Corp (rescinded)                                                    |
| `00 02 0B` | Denon DJ                                                                           |
| `00 02 0C` | Keith Robert Murray                                                                |
| `00 02 0D` | Google                                                                             |
| `00 02 0E` | ISP Technologies                                                                   |
| `00 02 0F` | Abstrakt Instruments LLC (rescinded)                                               |
| `00 02 10` | Meris LLC                                                                          |
| `00 02 11` | Sensorpoint LLC                                                                    |
| `00 02 12` | Hi-Z Labs                                                                          |
| `00 02 13` | Imitone                                                                            |
| `00 02 14` | Intellijel Designs Inc.                                                            |
| `00 02 15` | Dasz Instruments Inc.                                                              |
| `00 02 16` | Remidi                                                                             |
| `00 02 17` | Disaster Area Designs LLC                                                          |
| `00 02 18` | Universal Audio                                                                    |
| `00 02 19` | Carter Duncan Corp                                                                 |
| `00 02 1A` | Essential Technology                                                               |
| `00 02 1B` | Cantux Research LLC                                                                |
| `00 02 1C` | Hummel Technologies                                                                |
| `00 02 1D` | Sensel Inc                                                                         |
| `00 02 1E` | DBML Group                                                                         |
| `00 02 1F` | Madrona Labs                                                                       |
| --         | --                                                                                 |
| `00 20 00` | Dream SAS                                                                          |
| `00 20 01` | Strand Lighting                                                                    |
| `00 20 02` | Amek Div of Harman Industries                                                      |
| `00 20 03` | Casa Di Risparmio Di Loreto (rescinded)                                            |
| `00 20 04` | Bohm Electronic GmbH                                                               |
| `00 20 05` | Syntec Digital Audio (rescinded)                                                   |
| `00 20 06` | Trident Audio Developments                                                         |
| `00 20 07` | Real World Studio                                                                  |
| `00 20 08` | Evolution Synthesis, Ltd. (rescinded)                                              |
| `00 20 09` | Yes Technology                                                                     |
| `00 20 0A` | Audiomatica                                                                        |
| `00 20 0B` | Bontempi SpA (Sigma) / Farfisa                                                     |
| `00 20 0C` | F.B.T. Elettronica SpA                                                             |
| `00 20 0D` | MidiTemp GmbH                                                                      |
| `00 20 0E` | LA Audio (Larking Audio)                                                           |
| `00 20 0F` | Zero 88 Lighting Limited                                                           |
| `00 20 10` | Micon Audio Electronics GmbH                                                       |
| `00 20 11` | Forefront Technology                                                               |
| `00 20 12` | Studio Audio and Video Ltd. (rescinded)                                            |
| `00 20 13` | Kenton Electronics                                                                 |
| `00 20 14` | Celco / Electrosonic (rescinded)                                                   |
| `00 20 15` | ADB                                                                                |
| `00 20 16` | Marshall Products Limited                                                          |
| `00 20 17` | DDA                                                                                |
| `00 20 18` | BSS Audio Ltd.                                                                     |
| `00 20 19` | MA Lighting Technology                                                             |
| `00 20 1A` | Fatar SRL c/o Music Industries                                                     |
| `00 20 1B` | QSC Audio Products Inc. (rescinded)                                                |
| `00 20 1C` | Artisan Clasic Organ Inc.                                                          |
| `00 20 1D` | Orla Spa                                                                           |
| `00 20 1E` | Pinnacle Audio (Klark Teknik PLC)                                                  |
| `00 20 1F` | TC Electronics                                                                     |
| `00 20 20` | Doepfer Musikelektronik GmbH                                                       |
| `00 20 21` | Creative ATC / E-mu                                                                |
| `00 20 22` | Seyddo/Minami                                                                      |
| `00 20 23` | LG Electronics / Goldstar                                                          |
| `00 20 24` | Midisoft sas di M.Cima & C                                                         |
| `00 20 25` | Samick Musical Inst. Co. Ltd.                                                      |
| `00 20 26` | Penny and Giles (Bowthorpe PLC)                                                    |
| `00 20 27` | Acorn Computer                                                                     |
| `00 20 28` | LSC Electronics Pty. Ltd.                                                          |
| `00 20 29` | Focusrite / Novation EMS                                                           |
| `00 20 2A` | Samkyung Mechatronics                                                              |
| `00 20 2B` | Medeli Electronics Co.                                                             |
| `00 20 2C` | Charlie Lab SRL                                                                    |
| `00 20 2D` | Blue Chip Music Tech                                                               |
| `00 20 2E` | BEE OH Corp                                                                        |
| `00 20 2F` | LG Semicon America                                                                 |
| `00 20 30` | TESI                                                                               |
| `00 20 31` | EMAGIC                                                                             |
| `00 20 32` | Behringer GmbH                                                                     |
| `00 20 33` | Access Music Electronics                                                           |
| `00 20 34` | Synoptic                                                                           |
| `00 20 35` | Hanmesoft                                                                          |
| `00 20 36` | Terratec Electronic GmbH                                                           |
| `00 20 37` | Proel SpA                                                                          |
| `00 20 38` | IBK MIDI                                                                           |
| `00 20 39` | IRCAM                                                                              |
| `00 20 3A` | Propellerhead Software (rescinded)                                                 |
| `00 20 3B` | Red Sound Systems Ltd.                                                             |
| `00 20 3C` | Elektron ESI AB                                                                    |
| `00 20 3D` | Sintefex Audio                                                                     |
| `00 20 3E` | MAM (Music and More)                                                               |
| `00 20 3F` | Amsaro GmbH                                                                        |
| `00 20 40` | CDS Advanced Technology BV (Lanbox) (rescinded)                                    |
| `00 20 41` | Mode Machines (Touched By Sound GmbH)                                              |
| `00 20 42` | DSP Arts                                                                           |
| `00 20 43` | Phil Rees Music Tech                                                               |
| `00 20 44` | Stamer Musikanlagen GmbH (rescinded)                                               |
| `00 20 45` | Musical Muntaner S.A. dba Soundart                                                 |
| `00 20 46` | C-Mexx Software (rescinded)                                                        |
| `00 20 47` | Klavis Technologies                                                                |
| `00 20 48` | Noteheads AB (rescinded)                                                           |
| `00 20 49` | Algorithmix                                                                        |
| `00 20 4A` | Skrydstrup R&D (rescinded)                                                         |
| `00 20 4B` | Professional Audio Company                                                         |
| `00 20 4C` | NewWave Labs (MadWaves)                                                            |
| `00 20 4D` | Vermona (rescinded)                                                                |
| `00 20 4E` | Nokia                                                                              |
| `00 20 4F` | Wave Idea                                                                          |
| `00 20 50` | Hartmann GmbH                                                                      |
| `00 20 51` | Lion's Tracs (rescinded)                                                           |
| `00 20 52` | Analogue Systems (rescinded)                                                       |
| `00 20 53` | Focal-JMlab (rescinded)                                                            |
| `00 20 54` | Ringway Electronics (Chang-Zhou) Co. Ltd.                                          |
| `00 20 55` | Faith Technologies (Digiplug) (rescinded)                                          |
| `00 20 56` | Showworks (rescinded)                                                              |
| `00 20 57` | Manikin Electronic                                                                 |
| `00 20 58` | 1 Come Tech                                                                        |
| `00 20 59` | Phonic Corp (rescinded)                                                            |
| `00 20 5A` | Dolby Australia (Lake)                                                             |
| `00 20 5B` | Silansys Technologies (rescinded)                                                  |
| `00 20 5C` | Winbond Electronics (rescinded)                                                    |
| `00 20 5D` | Cinetix Medien und Interface GmbH                                                  |
| `00 20 5E` | A&G Soluzioni Digitali                                                             |
| `00 20 5F` | Sequentix Music Systems                                                            |
| `00 20 60` | Oram Pro Audio (rescinded)                                                         |
| `00 20 61` | Be4 Ltd. (rescinded)                                                               |
| `00 20 62` | Infection Music (rescinded)                                                        |
| `00 20 63` | Central Music Co. (CME)                                                            |
| `00 20 64` | genoQs Machines GmbH (rescinded)                                                   |
| `00 20 65` | Medialon                                                                           |
| `00 20 66` | Waves Audio Ltd.                                                                   |
| `00 20 67` | Jerash Labs (rescinded)                                                            |
| `00 20 68` | Da Fact                                                                            |
| `00 20 69` | Elby Designs                                                                       |
| `00 20 6A` | Spectral Audio (rescinded)                                                         |
| `00 20 6B` | Arturia                                                                            |
| `00 20 6C` | Vixid (rescinded)                                                                  |
| `00 20 6D` | C-Thru Music                                                                       |
| `00 20 6E` | Ya Horng Electronic Co. Ltd.                                                       |
| `00 20 6F` | SM Pro Audio (rescinded)                                                           |
| `00 20 70` | OTO MACHINES                                                                       |
| `00 20 71` | ELZAB S.A. (G LAB) (rescinded)                                                     |
| `00 20 72` | Blackstar Amplification Ltd.                                                       |
| `00 20 73` | M3i Technologies GmbH                                                              |
| `00 20 74` | Gemalto (from Xiring) (rescinded)                                                  |
| `00 20 75` | Prostage SL                                                                        |
| `00 20 76` | Teenage Engineering                                                                |
| `00 20 77` | Tobias Erichsen Consulting (rescinded)                                             |
| `00 20 78` | Nixer Ltd. (rescinded)                                                             |
| `00 20 79` | Hanpin Electron Co. Ltd.                                                           |
| `00 20 7A` | MIDI-hardware R.Sowa                                                               |
| `00 20 7B` | Beyond Music Industrial Ltd.                                                       |
| `00 20 7C` | Kiss Box B.V.                                                                      |
| `00 20 7D` | Misa Digital Technologies Ltd.                                                     |
| `00 20 7E` | AI Musics Technology Inc. (rescinded)                                              |
| `00 20 7F` | Serato Inc. LP                                                                     |
| --         | --                                                                                 |
| `00 21 00` | Limex (rescinded)                                                                  |
| `00 21 01` | Kyodday (Tokai) (rescinded)                                                        |
| `00 21 02` | Mutable Instruments                                                                |
| `00 21 03` | PreSonus Software Ltd.                                                             |
| `00 21 04` | Ingenico / Xiring                                                                  |
| `00 21 05` | Fairlight Instruments Pty Ltd. (rescinded)                                         |
| `00 21 06` | Musicom Lab                                                                        |
| `00 21 07` | Modal Electronics (Modulus/VacoLoco)                                               |
| `00 21 08` | RWA (Hong Kong) Limited (rescinded)                                                |
| `00 21 09` | Native Instruments                                                                 |
| `00 21 0A` | Naonext                                                                            |
| `00 21 0B` | MFB                                                                                |
| `00 21 0C` | Teknel Research                                                                    |
| `00 21 0D` | Ploytec GmbH                                                                       |
| `00 21 0E` | Surfin Kangaroo Studio                                                             |
| `00 21 0F` | Philips Electronics HK Ltd. (rescinded)                                            |
| `00 21 10` | ROLI Ltd.                                                                          |
| `00 21 11` | Panda-Audio Ltd.                                                                   |
| `00 21 12` | BauM Software                                                                      |
| `00 21 13` | Machinewerks Ltd. (rescinded)                                                      |
| `00 21 14` | Xiamen Elane Electronics (rescinded)                                               |
| `00 21 15` | Marshall Amplification PLC                                                         |
| `00 21 16` | Kiwitechnics Ltd.                                                                  |
| `00 21 17` | Rob Papen                                                                          |
| `00 21 18` | Spicetone OU (rescinded)                                                           |
| `00 21 19` | V3Sound                                                                            |
| `00 21 1A` | IK Multimedia                                                                      |
| `00 21 1B` | Novalia Ltd.                                                                       |
| `00 21 1C` | Modor Music                                                                        |
| `00 21 1D` | Ableton                                                                            |
| `00 21 1E` | Dtronics                                                                           |
| `00 21 1F` | ZAQ Audio                                                                          |
| `00 21 20` | Muabaobao Education Technology Co. Ltd.                                            |
| `00 21 21` | Flux Effects                                                                       |
| `00 21 22` | Audiothingies (MCDA)                                                               |
| `00 21 23` | Retrokits                                                                          |
| `00 21 24` | Morningstar FX Pte Ltd.                                                            |
| `00 21 25` | Hotone Audio                                                                       |
| `00 21 26` | Expressive                                                                         |
| `00 21 27` | Expert Sleepers Ltd.                                                               |
| `00 21 28` | Timecode-Vision Technology                                                         |
| `00 21 29` | Hornberg Research GbR                                                              |
| `00 21 2A` | Sonic Potions                                                                      |
| `00 21 2B` | Audiofront                                                                         |
| `00 21 2C` | Fred's Lab                                                                         |
| --         | --                                                                                 |
| `00 40 00` | Crimson Technology Inc.                                                            |
| `00 40 01` | Audiofront                                                                         |
| `00 40 02` | ?                                                                                  |
| `00 40 03` | D&M Holdings Inc.                                                                  |
