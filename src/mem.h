void *mi_alloc(int size);
void init_mem();

typedef char *address_t;
typedef enum {UNMAP, RDONLY, WRITE, MISS, INVALID} pagestate_t;
typedef struct{
	address_t addr;
	unsigned short int state;
	} mipage_t;
#define PAGESIZE 4096
#define MAXMEMSIZE 0x08000000
#define MAXPAGENUM (MAXMEMSIZE / PAGESIZE) 
#define STARTADDRESS 0x60000000
