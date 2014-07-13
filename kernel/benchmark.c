#include <debug.h>
#include <lib/string.h>

#define MAX_BSS_BYTES           5440
#define MEASURE_BLOCK_SIZE      1024

#define CYCLE_COUNT_REGADDR                         0xE0001004 
#define CONTROL_REGADDR                             0xE0001000
#define DEBUG_EXCEPTION_MONITOR_CONTROL_REGADDR     0xE000EDFC
 
static uint64_t *DWT_CYCCNT     = (uint64_t *) CYCLE_COUNT_REGADDR; 
static uint64_t *DWT_CONTROL    = (uint64_t *) CONTROL_REGADDR; 
static uint64_t *SCB_DEMCR      = (uint64_t *) DEBUG_EXCEPTION_MONITOR_CONTROL_REGADDR;

static int cnt_enable = 0;  

static char cblock[MEASURE_BLOCK_SIZE];

void dwt_cfg (void) 
{
    *SCB_DEMCR      = *SCB_DEMCR    | 0x01000000;
    *DWT_CONTROL    = *DWT_CONTROL  | 1 ; 

    if (!cnt_enable) {
        cnt_enable  = 1;        
        *DWT_CYCCNT = 0;  
    }   
}

uint64_t *fetch_cyccnt (void)
{
    dwt_cfg (); 

    return DWT_CYCCNT;
}

void measure_alignment_unit (void)
{   
    uint64_t start  = 0;
    uint64_t end    = 0;

    start = *fetch_cyccnt ();

    for (int i = 0; i < 4; ++i) {
    } 
    
    end = *fetch_cyccnt ();

    dbg_printf (DL_KDB, "The consumption of alignment unit: %ld\n", end - start);   
}

void measure_maximum_char_blocks (void)
{
    uint64_t start  = 0;
    uint64_t end    = 0;

    start = *fetch_cyccnt ();
    
    for (int i = 0; i < MAX_BSS_BYTES; ++i) {
    }   

    end = *fetch_cyccnt ();

    dbg_printf (DL_KDB, "The consumption of maximum of char blocks: %ld\n", end - start);
}

void init_char_block (int block_size)
{
    if (block_size > MEASURE_BLOCK_SIZE) {
        dbg_printf (DL_KDB, "[E]: The init size of char blocks is too big.\n");
    }

    for (int i = 0; i < block_size && block_size <= MEASURE_BLOCK_SIZE; ++i) {
        cblock[i] = 'A' + i;
    }
}

void stream_copy_access_unalignment (void)
{   
    uint64_t start   = 0;
    uint64_t end     = 0;
    uint64_t latency = 0;

    int base         = 2;
    int n_iterations = base;
    int n_case       = 8;

    for (int j = 0; j < n_case; ++j) {
        start = *fetch_cyccnt ();

        n_iterations = base << (j + 1);

        for (int i = 1; i <= n_iterations; ++i) {
            memcpy (cblock, cblock + n_iterations - 1, i);
        }

        end = *fetch_cyccnt ();

        latency = (end - start); // /n_iterations;

        dbg_printf (DL_KDB, "The block size is %d bytes, the latency is %ld \n", n_iterations, latency);
    }
}

void benchmark_main (void)
{
    measure_alignment_unit ();
    measure_maximum_char_blocks ();

    init_char_block (MEASURE_BLOCK_SIZE);
    stream_copy_access_unalignment ();

}

void benchmark_init (void)
{
	dbg_puts ("----------------------------------\n");
	dbg_puts ("-------|---------------------B----\n");
	dbg_puts ("-------|-------|-------------E----\n");
	dbg_puts ("-------|-------|---|---------N----\n");
	dbg_puts ("---|---|-------|---|---------C----\n");
	dbg_puts ("---|---|---|---|---|---------H----\n");
	dbg_puts ("---|---|---|---|---|---|-----M----\n");
	dbg_puts ("---|---|---|---|---|---|-----A----\n");
	dbg_puts ("---|---|---|---|---|---|-----R----\n");
	dbg_puts ("---|---|---|---|---|---|-----K----\n");
	dbg_puts ("----------------------------------\n");
	dbg_puts ("\n");
}

void benchmark_handler (void)
{
	benchmark_init (); 
	benchmark_main ();
}

