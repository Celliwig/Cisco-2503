// Boot ROM
//////////////////////////////////////////////////////////////////////////////////////////////
bool mem_bootrom_init(FILE *fhandle);
bool mem_bootrom_read_byte(unsigned address, unsigned int *value);
bool mem_bootrom_read_word(unsigned address, unsigned int *value);
bool mem_bootrom_read_long(unsigned address, unsigned int *value);

// NVRAM
//////////////////////////////////////////////////////////////////////////////////////////////
bool mem_nvram_read_byte(unsigned address, unsigned int *value);
bool mem_nvram_read_word(unsigned address, unsigned int *value);
bool mem_nvram_read_long(unsigned address, unsigned int *value);
bool mem_nvram_write_byte(unsigned address, unsigned int value);
bool mem_nvram_write_word(unsigned address, unsigned int value);
bool mem_nvram_write_long(unsigned address, unsigned int value);

// 68302
//////////////////////////////////////////////////////////////////////////////////////////////
bool io_68302_read_byte(unsigned address, unsigned int *value);
bool io_68302_read_word(unsigned address, unsigned int *value);
bool io_68302_read_long(unsigned address, unsigned int *value);
bool io_68302_write_byte(unsigned address, unsigned int value);
bool io_68302_write_word(unsigned address, unsigned int value);
bool io_68302_write_long(unsigned address, unsigned int value);

// System Registers
//////////////////////////////////////////////////////////////////////////////////////////////
bool io_system_read_byte(unsigned address, unsigned int *value);
bool io_system_read_word(unsigned address, unsigned int *value);
bool io_system_read_long(unsigned address, unsigned int *value);
bool io_system_write_byte(unsigned address, unsigned int value);
bool io_system_write_word(unsigned address, unsigned int value);
bool io_system_write_long(unsigned address, unsigned int value);

// Counter / Timer
//////////////////////////////////////////////////////////////////////////////////////////////
bool io_counter_read_byte(unsigned address, unsigned int *value);
bool io_counter_read_word(unsigned address, unsigned int *value);
bool io_counter_read_long(unsigned address, unsigned int *value);
bool io_counter_write_byte(unsigned address, unsigned int value);
bool io_counter_write_word(unsigned address, unsigned int value);
bool io_counter_write_long(unsigned address, unsigned int value);

// Dual UART
//////////////////////////////////////////////////////////////////////////////////////////////
bool io_duart_read_byte(unsigned address, unsigned int *value);
bool io_duart_read_word(unsigned address, unsigned int *value);
bool io_duart_read_long(unsigned address, unsigned int *value);
bool io_duart_write_byte(unsigned address, unsigned int value);
bool io_duart_write_word(unsigned address, unsigned int value);
bool io_duart_write_long(unsigned address, unsigned int value);

// Channel A: LANCE
//////////////////////////////////////////////////////////////////////////////////////////////
bool io_channela_read_byte(unsigned address, unsigned int *value);
bool io_channela_read_word(unsigned address, unsigned int *value);
bool io_channela_read_long(unsigned address, unsigned int *value);
bool io_channela_write_byte(unsigned address, unsigned int value);
bool io_channela_write_word(unsigned address, unsigned int value);
bool io_channela_write_long(unsigned address, unsigned int value);

// Channel B: LANCE / Serial
//////////////////////////////////////////////////////////////////////////////////////////////
bool io_channelb_read_byte(unsigned address, unsigned int *value);
bool io_channelb_read_word(unsigned address, unsigned int *value);
bool io_channelb_read_long(unsigned address, unsigned int *value);
bool io_channelb_write_byte(unsigned address, unsigned int value);
bool io_channelb_write_word(unsigned address, unsigned int value);
bool io_channelb_write_long(unsigned address, unsigned int value);
