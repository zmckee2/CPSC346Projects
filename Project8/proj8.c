/*
Class: CPSC 346-02
Team Member 1: Zach McKee
Team Member 2: N/A
GU Username of project lead: zmckee
Pgm Name: proj8.c
Pgm Desc: This program simulates shared memory with BACKING_STORE.bin as
		  secondary storage and addresses.txt as logical addresses

Usage: ./a.out BACKING_STORE.bin addresses.txt
	  Make sure BACKING_STORE.bin and addresses.txt are in the same directory

Basics of program needed to complete assignment. 
This example processes the first logical address stored in addresses.txt.
It translates the address to a physical address and looks up that physical address in BACKING_STORE.bin
The output is identical to the first line of correct.txt
usage: ./a.out BACKING_STORE.bin addresses.txt
BACKING_STORE.bin and addresses.txt must bin the same directory as the executable

Your task is to complete the program for all virtual adresses in addresses.txt.  Think of a loop reading addresses.txt 
as a memory bus passsing logical addresses to a program.   

See Figure 8.34 on p. 433.   The page number/offset block is what comes from addresses.txt.   
Here are the conditions that must be accounted for:

1. logical addreess is in TLB
2. logical address in not in TLB but is in memory
3. logical address is neither in TLB nor in memory



while(there are more logical addresses)
{
 extract the page number and offset from the logical address. 
 Search the TLB for the page number.
 If the page number is in the TLB, add the offset to the frame number found in the TLB. This is the physical address. 
   Use it to extract data from memory.
   Print out according to instructions in the text. 
 If the page number is not in the TLB, use the page number as an offset into the page table.
 If the page number is in the page table, put the page number and the frame number in the TLB. Add the offset to the
   page frame number found in the page table.  This is the physical address.  Use it to extract data from memory. 
 If the page number is not in the page table, bring the page in from BACKING_STORE.bin. You do this by translating the 
   address, just as I do in the example below. Add it to memory.  Update the page table and TLB.   
   Print out according to instructions in the text. 
}

This program simulates demand paging.  This means that memory is initially empty.  The program will page fault until
memory is full.  Since memory is the same size as BACKING_STORE, you do not have to handle page replacement. TLB is 
another story.  If you access a page that is not in TLB and TLB is full, you must evict an entry in the TLB.  
This means you need an eviction strategy.  Use LRU.  Include an integer in the TLB that will store the most recent 
address reference.  For example, there are 1000 addresses stored in addresses.txt.  If you get a TLB  hit 
(or store an entry in the TBL) on 77th counting from 0, store 77 in the TLB struct for that page.  If you need to evict
a page from the TLB, overwrite the TLB entry with the lowest associated number. 
*/

#include <stdio.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>

#define TLB_SIZE 16
#define PAGES 256
#define PAGE_MASK 255
#define PAGE_SIZE 256
#define OFFSET_BITS 8
#define OFFSET_MASK 255
#define MEMORY_SIZE PAGES *PAGE_SIZE

/*
********Important Types, Data Structures and Variables************************ 
Structs
tlb: struct representing an entry in translation lookaside buffer 

Storage
backing: pointer to memory mapped secondary storage
main_memory: array of integers simulating RAM

Tables
tlb: array of type tlb representing the translation lookaside buffer
pagetable: array of signed chars simulating the page table

Counters
numPageRefs: number of page table references
numPageFaults: counts number of page faults
numTLBRefs: number of TLB tries
numTLBHits: number of TLB references that resulted in a hit

Addresses 
logical_address: address read from addresses.txt, simulating the address bus 
physical_page: page frame number
physical_address: bytes from the 0th byte of RAM, i.e., the actual physical address 
logical_page: page table number
offset: displacement within page table/frame 

Output
Virtual address: logical_address, in Addresses, above
Physical address: physical_address in Addresses, above 
value: value stored in main_memory at physical_page displacement plus offset
*/

struct tlbentry
{
	unsigned char page_number;
	unsigned char frame_number;
	int tlb_ref;
};
typedef struct tlbentry tlbentry;

int main(int argc, char *argv[])
{
	int pagetable[PAGES];
	tlbentry tlb[TLB_SIZE];
	signed char main_memory[MEMORY_SIZE];
	signed char *backing;
	int logical_address;
	int offset;
	int logical_page;
	int physical_page;
	int physical_address;
	int numPageFaults = 0;
	int numPageRefs = 0;
	int numTLBRefs = 0;
	int numTLBHits = 0;
	int lastNewPhysicalPage = 0;
	int pageTableHit = 0;
	int tlbHit = 0;
	int numTranslatedAddresses = 0;

	//open simulation of secondary storage
	const char *backing_filename = argv[1]; //BACKING_STORE.bin
	int backing_fd = open(backing_filename, O_RDONLY);
	backing = mmap(0, MEMORY_SIZE, PROT_READ, MAP_PRIVATE, backing_fd, 0); //backing can be accessed as an array

	//open simulation of address bus and read the first line
	FILE *ifp = fopen(argv[2], "r"); //addresses.txt
	while (fscanf(ifp, "%d", &logical_address) > 0)
	{
		//extract low order 8 bits from the logical_address. This is the offset
		offset = logical_address & OFFSET_MASK;

		//extract bits 8 through 15. This is the page number gotten by shifting right 8 bits
		logical_page = (logical_address >> OFFSET_BITS) & PAGE_MASK;

		for(int i = 0; i < TLB_SIZE; i++) {
			if(/*tlb[i] != NULL && */tlb[i].page_number == logical_page){
				physical_page = tlb[i].frame_number;
				tlbHit = 1;
				tlb[i].tlb_ref = numTranslatedAddresses;
				numTLBHits++;
				break;
			} else {
				tlbHit = 0;
			}
		}
		numTLBRefs++;
		//No TLB hit, run through page table
		if(tlbHit == 0){
			//Check the page table to see if the logical page is already in it
			for (int i = 0; i < PAGES; i++)
			{
				//If the page is already in the table, change the current page
				//whatever physical page number it should be
				if (pagetable[i] == logical_page)
				{
					//We found the page, load its data and put it into the TLB
					physical_page = i;
					pageTableHit = 1;
					//Add the page to the TLB
					int oldest = 0;
					for(int j = 0; j < TLB_SIZE; j++) {
						/*if(tlb[j] == NULL){
							oldest = j;
							break;
						}*/
						if(tlb[j].tlb_ref < tlb[oldest].tlb_ref) {
							oldest = j;
						}
					}
					/*if(tlb[oldest] == NULL) {
						tlbentry newEntry;
						newEntry.page_number = logical_page;
						newEntry.frame_number = i;
						newEntry.tlb_ref = numTranslatedAddresses;
						tlb[oldest] = newEntry;
					} else {*/
						tlb[oldest].page_number = logical_page;
						tlb[oldest].frame_number = i;
						tlb[oldest].tlb_ref = numTranslatedAddresses;
					//}
					break;
				}
				//Else add it to the table later
				else
				{
					pageTableHit = 0;
				}
			}
		}
		//We just looked through the page table, add one
		//to the page table lookups
		numPageRefs++;
		//If the page was in the table, we don't need to reload it from memory
		//just print out the value and physical address
		if (pageTableHit || tlbHit)
		{
			physical_address = (physical_page << OFFSET_BITS) | offset;
			signed char value = main_memory[physical_page * PAGE_SIZE + offset];
			printf("Virtual address: %d Physical address: %d Value: %d\n",
				   logical_address, physical_address, value);
			
		}
		//Else load the page from memory, then print physical
		//address and value
		else
		{
			numPageFaults++;
			physical_page = lastNewPhysicalPage;
			//Add the page to the page table
			pagetable[lastNewPhysicalPage] = logical_page;
			//Add the page to the TLB
			int oldest = 0;
			for(int j = 0; j < TLB_SIZE; j++) {
				/*if(tlb[j] == NULL){
					oldest = j;
					break;
				}*/
				if(tlb[j].tlb_ref < tlb[oldest].tlb_ref) {
					oldest = j;
				}
			}
			/*if(tlb[oldest] == NULL) {
				tlbentry newEntry;
				newEntry.page_number = logical_page;
				newEntry.frame_number = lastNewPhysicalPage;
				newEntry.tlb_ref = numTranslatedAddresses;
				tlb[oldest] = newEntry;
			} else {*/
				tlb[oldest].page_number = logical_page;
				tlb[oldest].frame_number = lastNewPhysicalPage;
				tlb[oldest].tlb_ref = numTranslatedAddresses;
			//}
			//copy from secondary storage to simulated RAM. The address on secondary storage
			//as an offset into the memory map, backing, computing by multiplying the logical
			//page number by 256 and adding the offset
			memcpy(main_memory + physical_page * PAGE_SIZE,
				   backing + logical_page * PAGE_SIZE, PAGE_SIZE);

			//Shift the physical page left 8 bits and or with the offset
			physical_address = (physical_page << OFFSET_BITS) | offset;

			//extract the value store at offset bytes within the page
			signed char value = main_memory[physical_page * PAGE_SIZE + offset];

			printf("Virtual address: %d Physical address: %d Value: %d\n",
				   logical_address, physical_address, value);
			lastNewPhysicalPage++;
		}
		numTranslatedAddresses++;
	}

	float percentPageFault = ((float)numPageFaults / (float)numPageRefs);
	float percentTLBRate = ((float)numTLBHits/ (float)numTLBRefs);
	printf("Number of Translated Addresses = %d\n", numTranslatedAddresses);
	printf("Page Faults = %d\n", numPageFaults);
	printf("Page Fault Rate = %.3f\n", percentPageFault);
	printf("TLB hits = %d\n", numTLBHits);
	printf("TLB Hit Rate = %.3f\n", percentTLBRate);

	return 0;
}
