
#include <windows.h>
#include <stdio.h>
#include <string.h>

#include "types.h"
#include "cpu.h"

static inline unsigned long long rdtsc_diff() {
	unsigned long long ret, ret2;
	unsigned eax, edx;
	__asm__ volatile("rdtsc" : "=a" (eax), "=d" (edx));
	ret  = ((unsigned long long)eax) | (((unsigned long long)edx) << 32);
	__asm__ volatile("rdtsc" : "=a" (eax), "=d" (edx));
	ret2  = ((unsigned long long)eax) | (((unsigned long long)edx) << 32);
	return ret2 - ret;
}

static inline unsigned long long rdtsc_diff_vmexit() {
	unsigned long long ret, ret2;
	unsigned eax, edx;
	__asm__ volatile("rdtsc" : "=a" (eax), "=d" (edx));
	ret  = ((unsigned long long)eax) | (((unsigned long long)edx) << 32);
	/* vm exit forced here. it uses: eax = 0; cpuid; */
	__asm__ volatile("cpuid" : /* no output */ : "a"(0x00));
	/**/
	__asm__ volatile("rdtsc" : "=a" (eax), "=d" (edx));
	ret2  = ((unsigned long long)eax) | (((unsigned long long)edx) << 32);
	return ret2 - ret;
}

static inline void cpuid_vendor_00(char * vendor) {
	int ebx, ecx, edx;

	__asm__ volatile("cpuid" \
			: "=b"(ebx), \
			  "=c"(ecx), \
			  "=d"(edx) \
			: "a"(0x00));
	sprintf(vendor  , "%c%c%c%c", ebx, (ebx >> 8), (ebx >> 16), (ebx >> 24));
	sprintf(vendor+4, "%c%c%c%c", edx, (edx >> 8), (edx >> 16), (edx >> 24));
	sprintf(vendor+8, "%c%c%c%c", ecx, (ecx >> 8), (ecx >> 16), (ecx >> 24));
	vendor[12] = 0x00;
}

static inline int cpuid_hv_bit() {
	int ecx;
	__asm__ volatile("cpuid" \
			: "=c"(ecx) \
			: "a"(0x01));
	return (ecx >> 31) & 0x1;
}

int cpu_rdtsc() {
	int i;
	unsigned long long diff, avg = 0;
	for (i = 0; i < 10; i++) {
		diff = rdtsc_diff();
		avg = avg + diff;
		Sleep(500);
	}
	avg = avg / 10;
	return (avg < 750 && avg > 0) ? FALSE : TRUE;
}

int cpu_rdtsc_force_vmexit() {
	int i;
	unsigned long long diff, avg = 0;
	for (i = 0; i < 10; i++) {
		diff = rdtsc_diff_vmexit();
		avg = avg + diff;
		Sleep(500);
	}
	avg = avg / 10;
	return (avg < 1000 && avg > 0) ? FALSE : TRUE;
}

int cpu_hv() {
	return cpuid_hv_bit() ? TRUE : FALSE;
}

void cpu_write_vendor(char * vendor) {
	cpuid_vendor_00(vendor);
}

int cpu_known_vm_vendors(char * vendor) {
	const int count = 4;
	int i;
	string strs[count];
	strs[0] = "KVMKVMKVMKVM";
	strs[1] = "Microsoft Hv";
	strs[2] = "VMwareVMware";
	strs[3] = "XenVMMXenVMM";
	for (i = 0; i < count; i++) {
		if (!memcmp(vendor, strs[i], 12)) return TRUE;
	}
	return FALSE;
}

