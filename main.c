//#include "all.h"
#define videomaxline 25
#define videomaxcol 80

#define BACKSPACE 0x08
#define ENTER 0x13

#define TRUE 1
#define FALSE 0

#define DEBUG 24

struct IDT
{
	unsigned short offsetl;
	unsigned short selector;
	unsigned short type;
	unsigned short offseth;

}__attribute__((packed));

struct IDTR
{
	unsigned short size;
	unsigned int addr;

}__attribute__((packed));

unsigned char keyboard[videomaxcol];
unsigned short kindex;

unsigned short curline;
unsigned short curcol;

struct IDT inttable[3];
struct IDTR idtr = { 256 * 8 - 1,0 };

unsigned char keyt[2] = { 'A', 0 };
unsigned char keybuf;

void shell();
void translate_shell();
void kprintf(char* str, int line, int col);
void kprintf_line_clear(int line, int col);
void kprintf_clear_all();
int kstrcmp(char* str1, char* str2);
int kstrlen(char* str1);
void init_intdesc();
void idt_ignore();
void idt_timer();
void idt_keyboard();
unsigned char transScan(unsigned char target);
void sh_clear();
void sh_version();
void egg();
void open();

void main()
{
	kprintf("C language is able to use", 10, 10);
	
	//init_intdesc();

	//kprintf_clear_all();
	//shell();

}



void kprintf(char* str, int line, int col) // str ���ڸ� line ��° �ٿ� ����ϴ� �Լ�
{
	char *video = (char*)(0xB8000 + 160 * line + 2*col);

	for (int i = 0; str[i] != 0; i++)
	{
		*video++ = str[i];
		*video++ = 0x03; // ù °�� ����, �� ���� ���� ��ü ��
	}

	return;
}



void kprintf_line_clear(int line, int col)
{
	char *video = (char*)(0xB8000 + 160 * line + 2*col);
	for (int i = 0; i < 160-2*col ; i++)
	{
		*video++ = 0;
		*video++ = 0x03;
	}
}



void kprintf_clear_all()
{
	for (int i = 0; i < videomaxline; i++)
		kprintf_line_clear(i,0);
}



int kstrcmp(char* str1, char* str2)
{
	for (int i = 0; i < kstrlen(str1); i++)
		if (str1[i] != str2[i]) return FALSE;
	return TRUE;
}



int kstrlen(char* str1)
{
	int i = 0;
	while (str1[i] != 0) i++;
	return i;
}



void init_intdesc()
{

	int i,j;
	unsigned int ptr;
	unsigned short *isr;

	{  // 0x00 : isr_ignore
		ptr = (unsigned int)idt_ignore;
		inttable[0].selector = (unsigned short)0x08;
		inttable[0].type = (unsigned short)0x8E00;
		inttable[0].offsetl = (unsigned short)(ptr & 0xFFFF);
		inttable[0].offseth = (unsigned short)(ptr >> 16);

	}

	{  // 0x01 : isr_timer
		ptr = (unsigned int)idt_timer;
		inttable[1].selector = (unsigned short)0x08;
		inttable[1].type = (unsigned short)0x8E00;
		inttable[1].offsetl = (unsigned short)(ptr & 0xFFFF);
		inttable[1].offseth = (unsigned short)(ptr >> 16);

	}

	{  // 0x02 : isr_keyboard
		ptr = (unsigned int)idt_keyboard;
		inttable[2].selector = (unsigned short)0x08;
		inttable[2].type = (unsigned short)0x8E00;
		inttable[2].offsetl = (unsigned short)(ptr & 0xFFFF);
		inttable[2].offseth = (unsigned short)(ptr >> 16);

	}

	// �����ּ� 0x0 ������ ISR ��ġ ����

	for (i = 0; i < 256; i++)
	{
		isr = (unsigned short*)(0x0 + i * 8);
		*isr = inttable[0].offsetl;
		*(isr + 1) = inttable[0].selector;
		*(isr + 2) = inttable[0].type;
		*(isr + 3) = inttable[0].offseth;

	}

	// Ÿ�̸� ISR ���
	{
		isr = (unsigned short*)(0x0 + 8 * 0x20);
		*isr = inttable[1].offsetl;
		*(isr + 1) = inttable[1].selector;
		*(isr + 2) = inttable[1].type;
		*(isr + 3) = inttable[1].offseth;
	}

	// Ű���� ISR ���

	{
		isr = (unsigned short*)(0x0 + 8 * 0x21);
		*isr = inttable[2].offsetl;
		*(isr + 1) = inttable[2].selector;
		*(isr + 2) = inttable[2].type;
		*(isr + 3) = inttable[2].offseth;

		kindex = 0;
	
		for (int i = 0; i < videomaxcol; i++)
			keyboard[i] = 0;
	}

	// Ű���� �۵�
	__asm__ __volatile__
	(
		"mov al, 0xAE;"
		"out 0x64, al;"
	);
	//  ���ͷ�Ʈ �۵� ����

	__asm__ __volatile__("mov eax, %0"::"r"(&idtr));
	__asm__ __volatile__("lidt [eax]");
	__asm__ __volatile__("mov al,0xFC");
	__asm__ __volatile__("out 0x21,al");
	__asm__ __volatile__("sti");

	return;


}



void idt_ignore()
{

	__asm__ __volatile__
	(
		"push gs;"
		"push fs;"
		"push es;"
		"push ds;"
		"pushad;"
		"pushfd;"
		"mov al, 0x20;"
		"out 0x20, al;"
	);

	kprintf("idt_ignore", 5, 40);
	
	__asm__ __volatile__
	(
		"popfd;"
		"popad;"
		"pop ds;"
		"pop es;"
		"pop fs;"
		"pop gs;"
		"leave;"
		"nop;"
		"iretd;"
	);
	
	
}



void idt_timer()
{

	__asm__ __volatile__
	(
		"push gs;"
		"push fs;"
		"push es;"
		"push ds;"
		"pushad;"
		"pushfd;"
		"mov al, 0x20;"
		"out 0x20, al;"
		
	);

	kprintf(keyt, videomaxline-1, videomaxcol-1);
	keyt[0]++;

	__asm__ __volatile__
	(
		"popfd;"
		"popad;"
		"pop ds;"
		"pop es;"
		"pop fs;"
		"pop gs;"
		"leave;"
		"nop;"
		"iretd;"
	);


}



void idt_keyboard()
{

	__asm__ __volatile__
	(
		"push gs;"
		"push fs;"
		"push es;"
		"push ds;"
		"pushad;"
		"pushfd;"
		"xor al,al;"
		"in al, 0x60;"
	);

	__asm__ __volatile__("mov %0, al;" :"=r"(keybuf) );
	
	keybuf = transScan(keybuf);

	if (keybuf == BACKSPACE && kindex != 0) // �齺���̽� �Է�
		keyboard[--kindex] = 0;
	else if (keybuf != 0xFF && keybuf != BACKSPACE)
		keyboard[kindex++] = keybuf;

	__asm__ __volatile__
	(
		"mov al, 0x20;"
		"out 0x20, al;"
	);

	__asm__ __volatile__
	(
		"popfd;"
		"popad;"
		"pop ds;"
		"pop es;"
		"pop fs;"
		"pop gs;"
		"leave;"
		"nop;"
		"iretd;"
	);

}



unsigned char transScan(unsigned char target)
{
	unsigned char result;

	switch (target) // scan code set 1 ����
	{
	case 0x1E: result = 'a'; break;
	case 0x30: result = 'b'; break;
	case 0x2E: result = 'c'; break;
	case 0x20: result = 'd'; break;
	case 0x12: result = 'e'; break;
	case 0x21: result = 'f'; break;
	case 0x22: result = 'g'; break;
	case 0x23: result = 'h'; break;
	case 0x17: result = 'i'; break;
	case 0x24: result = 'j'; break;
	case 0x25: result = 'k'; break;
	case 0x26: result = 'l'; break;
	case 0x32: result = 'm'; break;
	case 0x31: result = 'n'; break;
	case 0x18: result = 'o'; break;
	case 0x19: result = 'p'; break;
	case 0x10: result = 'q'; break;
	case 0x13: result = 'r'; break;
	case 0x1F: result = 's'; break;
	case 0x14: result = 't'; break;
	case 0x16: result = 'u'; break;
	case 0x2F: result = 'v'; break;
	case 0x11: result = 'w'; break;
	case 0x2D: result = 'x'; break;
	case 0x15: result = 'y'; break;
	case 0x2C: result = 'z'; break;
	case 0x39: result = ' '; break; // �����̽�
	case 0x0E: result = BACKSPACE; break; // �齺���̽� �ƽ�Ű�ڵ� = 8
	case 0x1C: result = ENTER; break; // Enter key
	default: result = 0xFF; break; 
		// �����ȵ� ���� �����Ѵ�. �����ڴ� 0xFF

	}

	return result;

}



void sh_clear()
{
	kprintf_clear_all();
	curline = -1;
}



void sh_version()
{
	kprintf("Sparkle [version 0.0.3]", ++curline, 0);
	kprintf("special thanks: gold_z", ++curline, 0);
}



void egg()
{
        kprintf("is hungry",++curline,0);
}



void shell()
{
	char path[] = "Sparkle>>";

	curline = 0;
	curcol = kstrlen(path);

	while (1)
	{
		__asm__ __volatile__("cli");

		if ( kindex != 0 && keyboard[kindex - 1] == ENTER)
		{	
			kprintf_line_clear(curline, curcol + kindex - 1);
			keyboard[kindex - 1] = 0;

			translate_shell();

			for (int i = 0; i < videomaxcol; i++)
				keyboard[i] = 0;

			curline++;
			kindex = 0;
		}

		kprintf(path,curline,0);
		kprintf_line_clear(curline, curcol+kindex);
		kprintf(keyboard, curline, curcol);

		__asm__ __volatile__("sti");
	}
}



void translate_shell()
{
	if (keyboard[0] == 0) { return; } // ���ɾ� ���� �׳� ENTER ħ
	if (kstrcmp(keyboard, "clear")) { sh_clear(); return; }
	if (kstrcmp(keyboard, "version")) { sh_version(); return; }
	
	kprintf("There is no such command.",++curline, 0);
}
