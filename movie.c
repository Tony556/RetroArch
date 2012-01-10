/*  SSNES - A Super Nintendo Entertainment System (SNES) Emulator frontend for libsnes.
 *  Copyright (C) 2010-2012 - Hans-Kristian Arntzen
 *
 *  Some code herein may be based on code found in BSNES.
 * 
 *  SSNES is free software: you can redistribute it and/or modify it under the terms
 *  of the GNU General Public License as published by the Free Software Found-
 *  ation, either version 3 of the License, or (at your option) any later version.
 *
 *  SSNES is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 *  without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *  PURPOSE.  See the GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along with SSNES.
 *  If not, see <http://www.gnu.org/licenses/>.
 */

#include "movie.h"
#include <stdio.h>
#include <stdlib.h>
#include "general.h"
#include "dynamic.h"

// CRC32 implementation taken from BSNES source :)

static const uint32_t crc32_table[256] = {
    0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f,
    0xe963a535, 0x9e6495a3, 0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
    0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91, 0x1db71064, 0x6ab020f2,
    0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
    0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9,
    0xfa0f3d63, 0x8d080df5, 0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
    0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b, 0x35b5a8fa, 0x42b2986c,
    0xdbbbc9d6, 0xacbcf940, 0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
    0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423,
    0xcfba9599, 0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
    0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d, 0x76dc4190, 0x01db7106,
    0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
    0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d,
    0x91646c97, 0xe6635c01, 0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
    0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950,
    0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
    0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7,
    0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
    0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa,
    0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
    0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81,
    0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
    0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683, 0xe3630b12, 0x94643b84,
    0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
    0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb,
    0x196c3671, 0x6e6b06e7, 0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
    0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5, 0xd6d6a3e8, 0xa1d1937e,
    0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
    0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55,
    0x316e8eef, 0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
    0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe, 0xb2bd0b28,
    0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
    0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f,
    0x72076785, 0x05005713, 0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
    0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242,
    0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
    0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69,
    0x616bffd3, 0x166ccf45, 0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
    0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc,
    0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
    0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693,
    0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
    0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d
};

uint32_t crc32_adjust(uint32_t crc32, uint8_t input)
{
   return ((crc32 >> 8) & 0x00ffffff) ^ crc32_table[(crc32 ^ input) & 0xff];
}

uint32_t crc32_calculate(const uint8_t *data, unsigned length)
{
   uint32_t crc32 = ~0;
   for (unsigned i = 0; i < length; i++)
      crc32 = crc32_adjust(crc32, data[i]);
   return ~crc32;
}

struct bsv_movie
{
   FILE *file;
   uint8_t *state;

   size_t *frame_pos; // A ring buffer keeping track of positions in the file for each frame.
   size_t frame_mask;
   size_t frame_ptr;

   bool playback;
   size_t min_file_pos;

   bool first_rewind;
   bool did_rewind;
};

#define BSV_MAGIC 0x42535631

#define MAGIC_INDEX 0
#define SERIALIZER_INDEX 1 // Not current used.
#define CRC_INDEX 2
#define STATE_SIZE_INDEX 3

// Convert to big-endian if needed
static inline uint32_t swap_if_big32(uint32_t val)
{
   if (is_little_endian()) // Little-endian
      return val;
   else
      return (val >> 24) | ((val >> 8) & 0xFF00) | ((val << 8) & 0xFF0000) | (val << 24);
}

static inline uint32_t swap_if_little32(uint32_t val)
{
   if (!is_little_endian()) // Big-endian
      return val;
   else
      return (val >> 24) | ((val >> 8) & 0xFF00) | ((val << 8) & 0xFF0000) | (val << 24);
}

static inline uint16_t swap_if_big16(uint16_t val)
{
   if (is_little_endian())
      return val;
   else
      return (val >> 8) | (val << 8);
}

static bool init_playback(bsv_movie_t *handle, const char *path)
{
   handle->playback = true;
   handle->file = fopen(path, "rb");
   if (!handle->file)
   {
      SSNES_ERR("Couldn't open BSV file \"%s\" for playback.\n", path);
      return false;
   }

   uint32_t header[4] = {0};
   if (fread(header, sizeof(uint32_t), 4, handle->file) != 4)
   {
      SSNES_ERR("Couldn't read movie header!\n");
      return false;
   }

   // Compatibility with old implementation that used incorrect documentation.
   if (swap_if_little32(header[MAGIC_INDEX]) != BSV_MAGIC && swap_if_big32(header[MAGIC_INDEX]) != BSV_MAGIC)
   {
      SSNES_ERR("Movie file is not a valid BSV1 file!\n");
      return false;
   }

   if (swap_if_big32(header[CRC_INDEX]) != g_extern.cart_crc)
   {
      SSNES_ERR("Cart CRC32s differ! Cannot play back!\n");
      return false;
   }

   uint32_t state_size = swap_if_big32(header[STATE_SIZE_INDEX]);

   // If we're playing back from the start, state_size is 0.
   if (state_size)
   {
      handle->state = (uint8_t*)malloc(state_size);
      if (!handle->state)
         return false;

      if (fread(handle->state, 1, state_size, handle->file) != state_size)
      {
         SSNES_ERR("Couldn't read state from movie.\n");
         return false;
      }

      if (psnes_serialize_size() != state_size)
      {
         SSNES_ERR("Movie format seems to have a different serializer version. Cannot continue.\n");
         exit(1);
      }

      // Unserialize to start playback.
      psnes_unserialize(handle->state, state_size);
   }

   handle->min_file_pos = sizeof(header) + state_size;

   return true;
}

static bool init_record(bsv_movie_t *handle, const char *path)
{
   handle->file = fopen(path, "wb");
   if (!handle->file)
   {
      SSNES_ERR("Couldn't open BSV \"%s\" for recording.\n", path);
      return false;
   }

   uint32_t header[4] = {0};

   // This value is supposed to show up as BSV1 in a HEX editor.
   header[MAGIC_INDEX] = swap_if_little32(BSV_MAGIC);

   header[CRC_INDEX] = swap_if_big32(g_extern.cart_crc);

   uint32_t state_size = psnes_serialize_size();

   header[STATE_SIZE_INDEX] = swap_if_big32(state_size);
   fwrite(header, 4, sizeof(uint32_t), handle->file);

   handle->state = (uint8_t*)malloc(state_size);
   if (!handle->state)
      return false;

   handle->min_file_pos = sizeof(header) + state_size;

   if (state_size > 0)
   {
      psnes_serialize(handle->state, state_size);
      fwrite(handle->state, 1, state_size, handle->file);
   }

   return true;
}

void bsv_movie_free(bsv_movie_t *handle)
{
   if (handle)
   {
      if (handle->file)
         fclose(handle->file);
      free(handle->state);
      free(handle->frame_pos);
      free(handle);
   }
}

bool bsv_movie_get_input(bsv_movie_t *handle, int16_t *input)
{
   if (fread(input, sizeof(int16_t), 1, handle->file) != 1)
      return false;

   *input = swap_if_big16(*input);
   return true;
}

void bsv_movie_set_input(bsv_movie_t *handle, int16_t input)
{
   input = swap_if_big16(input);
   fwrite(&input, sizeof(int16_t), 1, handle->file);
}

bsv_movie_t *bsv_movie_init(const char *path, enum ssnes_movie_type type)
{
   bsv_movie_t *handle = (bsv_movie_t*)calloc(1, sizeof(*handle));
   if (!handle)
      return NULL;

   if (type == SSNES_MOVIE_PLAYBACK)
   {
      if (!init_playback(handle, path))
         goto error;
   }
   else if (!init_record(handle, path))
      goto error;

   // Just pick something really large :D ~1 million frames rewind should do the trick.
   if (!(handle->frame_pos = (size_t*)calloc((1 << 20), sizeof(size_t))))
      goto error; 

   handle->frame_pos[0] = handle->min_file_pos;
   handle->frame_mask = (1 << 20) - 1;

   return handle;

error:
   bsv_movie_free(handle);
   return NULL;
}

void bsv_movie_set_frame_start(bsv_movie_t *handle)
{
   handle->frame_pos[handle->frame_ptr] = ftell(handle->file);
}

void bsv_movie_set_frame_end(bsv_movie_t *handle)
{
   handle->frame_ptr = (handle->frame_ptr + 1) & handle->frame_mask;

   handle->first_rewind = !handle->did_rewind;
   handle->did_rewind = false;
}

void bsv_movie_frame_rewind(bsv_movie_t *handle)
{
   handle->did_rewind = true;

   // If we're at the beginning ... :)
   if ((handle->frame_ptr <= 1) && (handle->frame_pos[0] == handle->min_file_pos))
   {
      handle->frame_ptr = 0;
      fseek(handle->file, handle->min_file_pos, SEEK_SET);
   }
   else
   {
      // First time rewind is performed, the old frame is simply replayed.
      // However, playing back that frame caused us to read data, and push data to the ring buffer.
      // Sucessively rewinding frames, we need to rewind past the read data, plus another.
      handle->frame_ptr = (handle->frame_ptr - (handle->first_rewind ? 1 : 2)) & handle->frame_mask;
      fseek(handle->file, handle->frame_pos[handle->frame_ptr], SEEK_SET);
   }

   // We rewound past the beginning. :O
   if (ftell(handle->file) <= (long)handle->min_file_pos)
   {
      // If recording, we simply reset the starting point. Nice and easy.
      if (!handle->playback)
      {
         fseek(handle->file, 4 * sizeof(uint32_t), SEEK_SET);
         psnes_serialize(handle->state, psnes_serialize_size());
         fwrite(handle->state, 1, psnes_serialize_size(), handle->file);
      }
      else
         fseek(handle->file, handle->min_file_pos, SEEK_SET);
   }
}
