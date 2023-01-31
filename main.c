#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "clown68000/clowncommon/clowncommon.h"
#include "clown68000/m68k.h"
#include "cJSON/cJSON.h"
#include "libdeflate/libdeflate.h"

/*#define OUTPUT_STUFF*/

static M68k_State m68k;
static cc_u8l ram[0x1000000];
static char decompression_buffer[16 * 1024 * 1024];

#ifdef OUTPUT_STUFF
static unsigned int uses_memory;
#endif

static void ErrorCallback(const char *format, va_list arg)
{
	vfprintf(stderr, format, arg);
	fputc('\n', stderr);
	fflush(stderr);
}

static cc_u16f ReadCallback(const void* const user_data, const cc_u32f address, const cc_bool do_high_byte, const cc_bool do_low_byte)
{
	cc_u16f value;

	(void)user_data;

#ifdef OUTPUT_STUFF
	++uses_memory;
#endif

	value = 0;

	if (do_high_byte)
		value |= ram[address + 0] << 8;

	if (do_low_byte)
		value |= ram[address + 1] << 0;

	return value;
}

static void WriteCallback(const void* const user_data, const cc_u32f address, const cc_bool do_high_byte, const cc_bool do_low_byte, const cc_u16f value)
{
	(void)user_data;

#ifdef OUTPUT_STUFF
	++uses_memory;
#endif

	if (do_high_byte)
		ram[address + 0] = value >> 8;

	if (do_low_byte)
		ram[address + 1] = value & 0xFF;
}

static cc_bool LoadFile(const char* const file_path, void** const file_buffer, size_t* const file_size)
{
	FILE* const file = fopen(file_path, "rb");

	if (file != NULL)
	{
		if (!fseek(file, 0, SEEK_END))
		{
			const long long_size = ftell(file);

			if (long_size != -1 && !fseek(file, 0, SEEK_SET))
			{
				*file_size = long_size;

				*file_buffer = malloc(*file_size);

				if (*file_buffer != NULL)
				{
					if (fread(*file_buffer, *file_size, 1, file) == 1)
					{
						fclose(file);
						return cc_true;
					}

					free(*file_buffer);
				}
			}
		}

		fclose(file);
	}

	return cc_false;
}

int main(const int argc, char** const argv)
{
	cc_u16f file_index;

	void *file_buffer;
	size_t file_size;

	static const char* const files[] = {
		/* Not working. */
/*		"ABCD.b.json.gz",
		"ASR.b.json.gz",
		"ASR.l.json.gz",
		"ASR.w.json.gz",
		"CHK.json.gz",
		"DIVU.json.gz",
		"NBCD.json.gz",
		"RESET.json.gz",
		"SBCD.json.gz",
*/
		/* Working. */
		"ADD.b.json.gz",
		"ADD.l.json.gz",
		"ADD.w.json.gz",
		"ADDA.l.json.gz",
		"ADDA.w.json.gz",
		"ADDX.b.json.gz",
		"ADDX.l.json.gz",
		"ADDX.w.json.gz",
		"AND.b.json.gz",
		"AND.l.json.gz",
		"AND.w.json.gz",
		"ANDItoCCR.json.gz",
		"ANDItoSR.json.gz",
		"ASL.b.json.gz",
		"ASL.l.json.gz",
		"ASL.w.json.gz",
		"Bcc.json.gz",
		"BCHG.json.gz",
		"BCLR.json.gz",
		"BSET.json.gz",
		"BSR.json.gz",
		"BTST.json.gz",
		"CLR.b.json.gz",
		"CLR.l.json.gz",
		"CLR.w.json.gz",
		"CMP.b.json.gz",
		"CMP.l.json.gz",
		"CMP.w.json.gz",
		"CMPA.l.json.gz",
		"CMPA.w.json.gz",
		"DBcc.json.gz",
		"DIVS.json.gz",
		"EOR.b.json.gz",
		"EOR.l.json.gz",
		"EOR.w.json.gz",
		"EORItoCCR.json.gz",
		"EORItoSR.json.gz",
		"EXG.json.gz",
		"EXT.l.json.gz",
		"EXT.w.json.gz",
		"JMP.json.gz",
		"JSR.json.gz",
		"LEA.json.gz",
		"LINK.json.gz",
		"LSL.b.json.gz",
		"LSL.l.json.gz",
		"LSL.w.json.gz",
		"LSR.b.json.gz",
		"LSR.l.json.gz",
		"LSR.w.json.gz",
		"MOVE.b.json.gz",
		"MOVE.l.json.gz",
		"MOVE.q.json.gz",
		"MOVE.w.json.gz",
		"MOVEA.l.json.gz",
		"MOVEA.w.json.gz",
		"MOVEfromSR.json.gz",
		"MOVEfromUSP.json.gz",
		"MOVEM.l.json.gz",
		"MOVEM.w.json.gz",
		"MOVEP.l.json.gz",
		"MOVEP.w.json.gz",
		"MOVEtoCCR.json.gz",
		"MOVEtoSR.json.gz",
		"MOVEtoUSP.json.gz",
		"MULS.json.gz",
		"MULU.json.gz",
		"NEG.b.json.gz",
		"NEG.l.json.gz",
		"NEG.w.json.gz",
		"NEGX.b.json.gz",
		"NEGX.l.json.gz",
		"NEGX.w.json.gz",
		"NOP.json.gz",
		"NOT.b.json.gz",
		"NOT.l.json.gz",
		"NOT.w.json.gz",
		"OR.b.json.gz",
		"OR.l.json.gz",
		"OR.w.json.gz",
		"ORItoCCR.json.gz",
		"ORItoSR.json.gz",
		"PEA.json.gz",
		"ROL.b.json.gz",
		"ROL.l.json.gz",
		"ROL.w.json.gz",
		"ROR.b.json.gz",
		"ROR.l.json.gz",
		"ROR.w.json.gz",
		"ROXL.b.json.gz",
		"ROXL.l.json.gz",
		"ROXL.w.json.gz",
		"ROXR.b.json.gz",
		"ROXR.l.json.gz",
		"ROXR.w.json.gz",
		"RTE.json.gz",
		"RTR.json.gz",
		"RTS.json.gz",
		"Scc.json.gz",
		"SUB.b.json.gz",
		"SUB.l.json.gz",
		"SUB.w.json.gz",
		"SUBA.l.json.gz",
		"SUBA.w.json.gz",
		"SUBX.b.json.gz",
		"SUBX.l.json.gz",
		"SUBX.w.json.gz",
		"SWAP.json.gz",
		"TAS.json.gz",
		"TRAP.json.gz",
		"TRAPV.json.gz",
		"TST.b.json.gz",
		"TST.l.json.gz",
		"TST.w.json.gz",
		"UNLINK.json.gz"
	};

	struct libdeflate_decompressor* const decompressor = libdeflate_alloc_decompressor();

	(void)argc;
	(void)argv;

	if (decompressor == NULL)
	{
		fputs("Failed to allocate the GZIP decompressor.\n", stderr);
	}
	else
	{
		M68k_SetErrorCallback(ErrorCallback);

		for (file_index = 0; file_index < CC_COUNT_OF(files); ++file_index)
		{
			if (!LoadFile(files[file_index], &file_buffer, &file_size))
			{
				fprintf(stderr, "Failed to read '%s'.\n", files[file_index]);
			}
			else
			{
				size_t decompressed_size;

				if (libdeflate_gzip_decompress(decompressor, file_buffer, file_size, decompression_buffer, sizeof(decompression_buffer), &decompressed_size) != LIBDEFLATE_SUCCESS)
				{
					fputs("Failed GZIP decompression.\n", stderr);
				}
				else
				{
					cJSON* const json = cJSON_ParseWithLength(decompression_buffer, decompressed_size);

					free(file_buffer);
					file_buffer = NULL;

					if (json == NULL)
					{
						fprintf(stderr, "Failed to parse '%s'.\n", files[file_index]);
					}
					else
					{
						int array_index;

						const int array_size = cJSON_GetArraySize(json);

					#ifdef OUTPUT_STUFF
						char path_buffer[0x100];
						FILE *initial_file, *final_file;

						strcpy(path_buffer, files[file_index]);
						memcpy(strrchr(path_buffer, '.'), "-initial.asm", sizeof("-initial.asm"));
						initial_file = fopen(path_buffer, "w");

						strcpy(path_buffer, files[file_index]);
						memcpy(strrchr(path_buffer, '.'), "-final.asm", sizeof("-final.asm"));
						final_file = fopen(path_buffer, "w");
					#endif

						for (array_index = 0; array_index < array_size; ++array_index)
						{
							unsigned int i;
							char reg[3];
							const cJSON *array_item;

							static const M68k_ReadWriteCallbacks callbacks = {ReadCallback, WriteCallback, NULL};

							const cJSON* const array_member = cJSON_GetArrayItem(json, array_index);
							const cJSON* const initial = cJSON_GetObjectItemCaseSensitive(array_member, "initial");
							const cJSON* const initial_prefetch = cJSON_GetObjectItemCaseSensitive(initial, "prefetch");
							const cJSON* const initial_ram = cJSON_GetObjectItemCaseSensitive(initial, "ram");
							const cJSON* const final = cJSON_GetObjectItemCaseSensitive(array_member, "final");
							const cJSON* const final_ram = cJSON_GetObjectItemCaseSensitive(final, "ram");

							/* I really do not care about exceptions right now. */
							if (cJSON_GetObjectItemCaseSensitive(final, "pc")->valuedouble == 0x1400 || cJSON_GetObjectItemCaseSensitive(final, "pc")->valuedouble == 0x2000)
								continue;

							reg[2] = '\0';

							reg[0] = 'd';
							reg[1] = '0';

							for (i = 0; i < 8; ++i)
							{
								m68k.data_registers[i] = cJSON_GetObjectItemCaseSensitive(initial, reg)->valuedouble;

								++reg[1];
							}

							reg[0] = 'a';
							reg[1] = '0';

							for (i = 0; i < 7; ++i)
							{
								m68k.address_registers[i] = cJSON_GetObjectItemCaseSensitive(initial, reg)->valuedouble;

								++reg[1];
							}

							m68k.status_register = cJSON_GetObjectItemCaseSensitive(initial, "sr")->valuedouble;

							m68k.supervisor_stack_pointer = cJSON_GetObjectItemCaseSensitive(initial, "ssp")->valuedouble;
							m68k.user_stack_pointer = cJSON_GetObjectItemCaseSensitive(initial, "usp")->valuedouble;

							m68k.address_registers[7] = (m68k.status_register & 0x2000) != 0 ? m68k.supervisor_stack_pointer : m68k.user_stack_pointer;

							m68k.program_counter = cJSON_GetObjectItemCaseSensitive(initial, "pc")->valuedouble;

							for (i = 0; (array_item = cJSON_GetArrayItem(initial_prefetch, i)) != NULL; ++i)
							{
								const cc_u16f value = (cc_u16f)array_item->valuedouble;

								ram[m68k.program_counter + i * 2 + 0] = value >> 8;
								ram[m68k.program_counter + i * 2 + 1] = value & 0xFF;
							}

							for (i = 0; (array_item = cJSON_GetArrayItem(initial_ram, i)) != NULL; ++i)
							{
								const cc_u32f address = cJSON_GetArrayItem(array_item, 0)->valuedouble;
								const cc_u8f value = cJSON_GetArrayItem(array_item, 1)->valuedouble;

								ram[address] = value;
							}

							m68k.instruction_register = cJSON_GetArrayItem(initial_prefetch, 0)->valuedouble;

						#ifdef OUTPUT_STUFF
							uses_memory = 0;
						#endif

							M68k_DoCycle(&m68k, &callbacks);

						#ifdef OUTPUT_STUFF
							if (uses_memory <= 1)
							{
								fprintf(initial_file, "\tdc.w\t$%X\n", (unsigned int)cJSON_GetArrayItem(initial_prefetch, 0)->valuedouble);
								fprintf(initial_file, "\tdc.l\t$%X\n", (unsigned int)cJSON_GetObjectItemCaseSensitive(initial, "usp")->valuedouble);
								fprintf(initial_file, "\tdc.w\t$%X\n", (unsigned int)cJSON_GetObjectItemCaseSensitive(initial, "sr")->valuedouble);
								fprintf(initial_file, "\tdc.l\t$%X\n", (unsigned int)cJSON_GetObjectItemCaseSensitive(initial, "d0")->valuedouble);
								fprintf(initial_file, "\tdc.l\t$%X\n", (unsigned int)cJSON_GetObjectItemCaseSensitive(initial, "d1")->valuedouble);
								fprintf(initial_file, "\tdc.l\t$%X\n", (unsigned int)cJSON_GetObjectItemCaseSensitive(initial, "d2")->valuedouble);
								fprintf(initial_file, "\tdc.l\t$%X\n", (unsigned int)cJSON_GetObjectItemCaseSensitive(initial, "d3")->valuedouble);
								fprintf(initial_file, "\tdc.l\t$%X\n", (unsigned int)cJSON_GetObjectItemCaseSensitive(initial, "d4")->valuedouble);
								fprintf(initial_file, "\tdc.l\t$%X\n", (unsigned int)cJSON_GetObjectItemCaseSensitive(initial, "d5")->valuedouble);
								fprintf(initial_file, "\tdc.l\t$%X\n", (unsigned int)cJSON_GetObjectItemCaseSensitive(initial, "d6")->valuedouble);
								fprintf(initial_file, "\tdc.l\t$%X\n", (unsigned int)cJSON_GetObjectItemCaseSensitive(initial, "d7")->valuedouble);
								fprintf(initial_file, "\tdc.l\t$%X\n", (unsigned int)cJSON_GetObjectItemCaseSensitive(initial, "a0")->valuedouble);
								fprintf(initial_file, "\tdc.l\t$%X\n", (unsigned int)cJSON_GetObjectItemCaseSensitive(initial, "a1")->valuedouble);
								fprintf(initial_file, "\tdc.l\t$%X\n", (unsigned int)cJSON_GetObjectItemCaseSensitive(initial, "a2")->valuedouble);
								fprintf(initial_file, "\tdc.l\t$%X\n", (unsigned int)cJSON_GetObjectItemCaseSensitive(initial, "a3")->valuedouble);
								fprintf(initial_file, "\tdc.l\t$%X\n", (unsigned int)cJSON_GetObjectItemCaseSensitive(initial, "a4")->valuedouble);
								fprintf(initial_file, "\tdc.l\t$%X\n", (unsigned int)cJSON_GetObjectItemCaseSensitive(initial, "a5")->valuedouble);
								fprintf(initial_file, "\tdc.l\t$%X\n", (unsigned int)cJSON_GetObjectItemCaseSensitive(initial, "a6")->valuedouble);
								fprintf(initial_file, "\tdc.l\t$%X\n\n", (unsigned int)cJSON_GetObjectItemCaseSensitive(initial, "ssp")->valuedouble);

								fprintf(final_file, "\tdc.l\t$%X\n", (unsigned int)cJSON_GetObjectItemCaseSensitive(final, "usp")->valuedouble);
								fprintf(final_file, "\tdc.w\t$%X\n", (unsigned int)cJSON_GetObjectItemCaseSensitive(final, "sr")->valuedouble);
								fprintf(final_file, "\tdc.l\t$%X\n", (unsigned int)cJSON_GetObjectItemCaseSensitive(final, "d0")->valuedouble);
								fprintf(final_file, "\tdc.l\t$%X\n", (unsigned int)cJSON_GetObjectItemCaseSensitive(final, "d1")->valuedouble);
								fprintf(final_file, "\tdc.l\t$%X\n", (unsigned int)cJSON_GetObjectItemCaseSensitive(final, "d2")->valuedouble);
								fprintf(final_file, "\tdc.l\t$%X\n", (unsigned int)cJSON_GetObjectItemCaseSensitive(final, "d3")->valuedouble);
								fprintf(final_file, "\tdc.l\t$%X\n", (unsigned int)cJSON_GetObjectItemCaseSensitive(final, "d4")->valuedouble);
								fprintf(final_file, "\tdc.l\t$%X\n", (unsigned int)cJSON_GetObjectItemCaseSensitive(final, "d5")->valuedouble);
								fprintf(final_file, "\tdc.l\t$%X\n", (unsigned int)cJSON_GetObjectItemCaseSensitive(final, "d6")->valuedouble);
								fprintf(final_file, "\tdc.l\t$%X\n", (unsigned int)cJSON_GetObjectItemCaseSensitive(final, "d7")->valuedouble);
								fprintf(final_file, "\tdc.l\t$%X\n", (unsigned int)cJSON_GetObjectItemCaseSensitive(final, "a0")->valuedouble);
								fprintf(final_file, "\tdc.l\t$%X\n", (unsigned int)cJSON_GetObjectItemCaseSensitive(final, "a1")->valuedouble);
								fprintf(final_file, "\tdc.l\t$%X\n", (unsigned int)cJSON_GetObjectItemCaseSensitive(final, "a2")->valuedouble);
								fprintf(final_file, "\tdc.l\t$%X\n", (unsigned int)cJSON_GetObjectItemCaseSensitive(final, "a3")->valuedouble);
								fprintf(final_file, "\tdc.l\t$%X\n", (unsigned int)cJSON_GetObjectItemCaseSensitive(final, "a4")->valuedouble);
								fprintf(final_file, "\tdc.l\t$%X\n", (unsigned int)cJSON_GetObjectItemCaseSensitive(final, "a5")->valuedouble);
								fprintf(final_file, "\tdc.l\t$%X\n", (unsigned int)cJSON_GetObjectItemCaseSensitive(final, "a6")->valuedouble);
								fprintf(final_file, "\tdc.l\t$%X\n\n", (unsigned int)cJSON_GetObjectItemCaseSensitive(final, "ssp")->valuedouble);
							}
						#endif

							#define TEST(OURS, THEIRS) \
							{ \
								const cc_u32f our_value = m68k.OURS; \
								const cc_u32f their_value = cJSON_GetObjectItemCaseSensitive(final, THEIRS)->valuedouble; \
							 \
								if (our_value != their_value) \
									fprintf(stderr, "[%s %4d] Mismatched %s - Expected 0x%" CC_PRIXFAST32 ", got 0x%" CC_PRIXFAST32 "\n", files[file_index], array_index + 1, THEIRS, their_value, our_value); \
							}

							reg[0] = 'd';
							reg[1] = '0';

							for (i = 0; i < 8; ++i)
							{
								TEST(data_registers[i], reg);

								++reg[1];
							}

							reg[0] = 'a';
							reg[1] = '0';

							for (i = 0; i < 7; ++i)
							{
								TEST(address_registers[i], reg);

								++reg[1];
							}

							TEST(status_register, "sr");

							if ((m68k.status_register & 0x2000) != 0)
							{
								TEST(address_registers[7], "ssp");
								TEST(user_stack_pointer, "usp");
							}
							else
							{
								TEST(supervisor_stack_pointer, "ssp");
								TEST(address_registers[7], "usp");
							}

							TEST(program_counter, "pc");

							for (i = 0; (array_item = cJSON_GetArrayItem(final_ram, i)) != NULL; ++i)
							{
								const cc_u32f address = cJSON_GetArrayItem(array_item, 0)->valuedouble;
								const cc_u8f value = cJSON_GetArrayItem(array_item, 1)->valuedouble;

								if (ram[address] != value)
									fprintf(stderr, "[%s %4d] Mismatched RAM 0x%08" CC_PRIXFAST32 " - Expected 0x%" CC_PRIXFAST8 ", got 0x%" CC_PRIXLEAST8 "\n", files[file_index], array_index + 1, address, value, ram[address]);
							}
						}

					#ifdef OUTPUT_STUFF
						fclose(initial_file);
						fclose(final_file);
					#endif
					}
				}

				free(file_buffer);
			}
		}

		libdeflate_free_decompressor(decompressor);
	}

	return EXIT_SUCCESS;
}
