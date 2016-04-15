
#define DWC_PR(r)							\
	printf("DWC2: REG["#r"]=%x\n", *((volatile unsigned int *)((r) | 0xb3500000)))

#define DWC_RR(_r) 	r##_r = (*((volatile unsigned int *)((_r) | 0xb3500000)))
#define DWC_P(v)	printf("DWC2: REG["#v"] = %x\n", r##v)



__attribute__((unused)) void dwc2_ep0_dump_regs(void) {
	unsigned int r0x800, r0x804, r0x808, r0x80c, r0x810, r0x814, r0x818, r0x81c;
	unsigned int r0x900, r0x908, r0x910, r0x914;
	unsigned int r0xb00, r0xb08, r0xb10, r0xb14;

	DWC_RR(0x900);
	DWC_RR(0x908);
	DWC_RR(0x910);
	DWC_RR(0x914);

	DWC_RR(0xb00);
	DWC_RR(0xb08);
	DWC_RR(0xb10);
	DWC_RR(0xb14);

	DWC_RR(0x800);
	DWC_RR(0x804);
	DWC_RR(0x808);
	DWC_RR(0x80c);
	DWC_RR(0x810);
	DWC_RR(0x814);
	DWC_RR(0x818);
	DWC_RR(0x81c);

	/*--------------------------*/

	DWC_P(0x800);
	DWC_P(0x804);
	DWC_P(0x808);
	DWC_P(0x80c);
	DWC_P(0x810);
	DWC_P(0x814);
	DWC_P(0x818);
	DWC_P(0x81c);

	DWC_P(0x900);
	DWC_P(0x908);
	DWC_P(0x910);
	DWC_P(0x914);

	DWC_P(0xb00);
	DWC_P(0xb08);
	DWC_P(0xb10);
	DWC_P(0xb14);


	/*--------------------------*/
}


__attribute__((unused)) void dwc2_dump_ep_regs(
	int epnum, const char *func, int line) {
	unsigned int r0x800, r0x804, r0x808, r0x80c, r0x810, r0x814, r0x818, r0x81c;
	unsigned int r0x900 = 0, r0x908 = 0, r0x910 = 0, r0x914 = 0, r0xb00 = 0, r0xb08 = 0, r0xb10 = 0, r0xb14 = 0;
	unsigned int r0x920 = 0, r0x928 = 0, r0x930 = 0, r0x934 = 0, r0xb20 = 0, r0xb28 = 0, r0xb30 = 0, r0xb34 = 0;
	unsigned int r0x940 = 0, r0x948 = 0, r0x950 = 0, r0x954 = 0, r0xb40 = 0, r0xb48 = 0, r0xb50 = 0, r0xb54 = 0;
	unsigned int r0x960 = 0, r0x968 = 0, r0x970 = 0, r0x974 = 0, r0xb60 = 0, r0xb68 = 0, r0xb70 = 0, r0xb74 = 0;

	DWC_RR(0x800);
	DWC_RR(0x804);
	DWC_RR(0x808);
	DWC_RR(0x80c);
	DWC_RR(0x810);
	DWC_RR(0x814);
	DWC_RR(0x818);
	DWC_RR(0x81c);

	switch (epnum) {
	case -1:
	case 0:
		DWC_RR(0x900);
		DWC_RR(0x908);
		DWC_RR(0x910);
		DWC_RR(0x914);
		DWC_RR(0xb00);
		DWC_RR(0xb08);
		DWC_RR(0xb10);
		DWC_RR(0xb14);

		if (epnum != -1)
			break;
	case 1:
		DWC_RR(0x920);
		DWC_RR(0x928);
		DWC_RR(0x930);
		DWC_RR(0x934);
		DWC_RR(0xb20);
		DWC_RR(0xb28);
		DWC_RR(0xb30);
		DWC_RR(0xb34);

		if (epnum != -1)
			break;
	case 2:
		DWC_RR(0x940);
		DWC_RR(0x948);
		DWC_RR(0x950);
		DWC_RR(0x954);
		DWC_RR(0xb40);
		DWC_RR(0xb48);
		DWC_RR(0xb50);
		DWC_RR(0xb54);

		if (epnum != -1)
			break;
	case 3:
		DWC_RR(0x960);
		DWC_RR(0x968);
		DWC_RR(0x970);
		DWC_RR(0x974);
		DWC_RR(0xb60);
		DWC_RR(0xb68);
		DWC_RR(0xb70);
		DWC_RR(0xb74);

		if (epnum != -1)
			break;
	default:
		return;
	}

	printf("=====%s:%d ep%d regs=====\n", func, line, epnum);
	DWC_P(0x800);
	DWC_P(0x804);
	DWC_P(0x808);
	DWC_P(0x80c);
	DWC_P(0x810);
	DWC_P(0x814);
	DWC_P(0x818);
	DWC_P(0x81c);

	switch (epnum) {
	case -1:
	case 0:
		DWC_P(0x900);
		DWC_P(0x908);
		DWC_P(0x910);
		DWC_P(0x914);
		DWC_P(0xb00);
		DWC_P(0xb08);
		DWC_P(0xb10);
		DWC_P(0xb14);

		if (epnum != -1)
			return;
	case 1:
		DWC_P(0x920);
		DWC_P(0x928);
		DWC_P(0x930);
		DWC_P(0x934);
		DWC_P(0xb20);
		DWC_P(0xb28);
		DWC_P(0xb30);
		DWC_P(0xb34);

		if (epnum != -1)
			return;
	case 2:
		DWC_P(0x940);
		DWC_P(0x948);
		DWC_P(0x950);
		DWC_P(0x954);
		DWC_P(0xb40);
		DWC_P(0xb48);
		DWC_P(0xb50);
		DWC_P(0xb54);

		if (epnum != -1)
			return;
	case 3:
		DWC_P(0x960);
		DWC_P(0x968);
		DWC_P(0x970);
		DWC_P(0x974);
		DWC_P(0xb60);
		DWC_P(0xb68);
		DWC_P(0xb70);
		DWC_P(0xb74);

		if (epnum != -1)
			return;
	default:
		return;
	}

}



