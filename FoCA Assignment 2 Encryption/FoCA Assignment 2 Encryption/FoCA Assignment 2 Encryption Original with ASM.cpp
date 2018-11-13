// The encryption program in C++ and ASM with a very simple encryption method - it simply adds 1 to the character.
// Last revised April 2018 by Josh Warburton

char EKey = 'g';

#define StudentName "Josh Warburton"
#define MAXCHARS 6

using namespace std;
#include <conio.h>		// for kbhit
#include <windows.h>
#include <string>       // for strings
#include <fstream>		// file I/O
#include <iostream>		// for cin >> and cout <<
#include <iomanip>		// for fancy output
#include "TimeUtils.h"  // for GetTime, GetDate, etc.

#define dollarchar '$'  // string terminator

char OChars[MAXCHARS],
EChars[MAXCHARS],
DChars[MAXCHARS] = "Soon!";	// Global Original, Encrypted, Decrypted character strings

							//----------------------------- C++ Functions ----------------------------------------------------------

void get_char(char& a_character)
{
	cin >> a_character;
	while (((a_character < '0') | (a_character > 'z')) && (a_character != dollarchar))
	{
		cout << "Alphanumeric characters only, please try again > ";
		cin >> a_character;
	}
}
//-------------------------------------------------------------------------------------------------------------

void get_original_chars(int& length)
{
	char next_char = ' ';
	length = 0;
	get_char(next_char);

	while ((length < MAXCHARS) && (next_char != dollarchar))
	{
		OChars[length++] = next_char;
		get_char(next_char);
	}
}

//---------------------------------------------------------------------------------------------------------------
//----------------- ENCRYPTION ROUTINES -------------------------------------------------------------------------

void encrypt_chars(int lengths, char EKey)
{
	char temp_char;					// Character temporary store
	int i;							// INDEX

	__asm {
		mov i, 0					// SETS INDEX TO 0
		mov esi, i					// MOVES THE INDEX INTO THE esi RESGISTER TO SET IT TO 0
		mov ebx, lengths			// MOVES THE VARIABLE THAT HOLDS THE LENGTH OF THE ARRAY INTO THE ebx REGISTER
		jmp checkend				// JUMPS STRAIGHT TO THE CHECKEND FUNCTION TO SEE IF THE STATEMENT FOR THE FOR LOOP IS TRUE OR FALSE
		forloop : mov esi, i		// MOVES THE CURRENT INDEX INTO THE esi REGISTER TO SET IT TO THE VALUE OF THE CURRENT CHAR
				  inc esi			// ADDS 1 TO THE INDEX
				  mov i, esi		// MOVES THE NEW INDEX VALUE BACK INTO THE esi REGISTER

		checkend : cmp esi, ebx		// CHECKS IF THE COUNTER HAS REACHED THE LENGTH OF THE ARRAY

		jge endforloop				// IF THE LENGTH OF THE ARRAY IS NOW LESS THAN THE COUNTER, JUMP TO endforloop, IF NOT THEN CONTINUE

		mov edx, i					// MOVES THE CURRENT INDEX INTO THE edx REGISTER

		movzx ecx, [edx + OChars]	// STORES THE ASCII CODE OF THE CURRENT CHARACTER INTO THE ecx REGISTER
		mov temp_char, cl			// STORES THE FIRST 8 BITS OF THE THE ECX REGISTER (ASCII CODE OF THE CURRENT CHAR) INTO THE temp_char VARIABLE

		push   eax					// MOVE eax TO THE TOP OF THE STACK 
		push   ecx					// MOVE ecx TO THE TOP OF THE STACK
		movzx  ecx, temp_char		// MOVE THE CHARACTER INTO THE ecx REGISTER
		lea    eax, EKey			// MOVE THE Ekey INTO THE eax REGISTER
		push ecx					// MOVE THE CHARACTER TO THE TOP OF THE STACK
		push eax					// MOVE THE Ekey TO THE TOP OF THE STACK
		call   encrypt_16			// RUNS THE ENCRYPTION FUNCTION
		mov    temp_char, al		// MOVE THE LEAST SIGNIFICANT BYTE OF THE eax REGISTER INTO temp_char

		add esp, 8					// ADDS 8 TO THE STACK POINTER IN ORDER TO REMOVE IT FROM THE STACK
		pop    ecx					// RESTORE THE ORIGINAL VALUE OF ECX
		pop    eax					// RESTORE THE ORIGINAL VALUE OF EAX
			
		mov cl, temp_char			// MOVES THE CURRENT CHARACTER (ASCII CODE) INTO THE cl (FIRST 8 BITS OF THE ecx REGISTER) REGISTER
		mov[edx + EChars], cl		// MOVES THE ASCII CODE STORED IN cl INTO THE Encrypted Characters ARRAY AT THE POSITION OF THE CURRENT INDEX
		jmp forloop					// JUMPS BACK TO THE STATEMENT OF THE FOR LOOP


		endforloop :				// USED TO BREAK OUT OF THE FOR LOOP WHEN THE STATEMENT BECOMES FALSE

	}

	return;

	__asm {

	encrypt_16:

		push ebp					// MOVES THE BASE POINTER TO THE TOP OF THE STACK
		mov ebp, esp				// MOVE THE STACK POINTER INTO THE BASE POINTER

		mov ecx, [ebp + 12]			// ADDS 12 (FROM THE BASE POINTER) TO ecx IN ORDER TO REMOVE IT FROM THE STACK
		mov eax, [ebp + 8]			// ADDS 8 (FROM THE BASE POINTER) TO eax IN ORDER TO REMOVE IT FROM THE STACK
		push ebx					// MOVE ebx REGISTER TO THE TOP OF THE STACK
		push edx					// MOVE edx TO THE TOP OF THE STACK
		push ecx					// MOVE Char TO THE TOP OF THE STACK
		movzx edx, byte ptr[eax]	// MOVE 8 LEAST SIGNIFICANT BITS FROM THE Ekey INTO THE EDX REGISTER
		and edx, 0x43				// PERFORMS A LOGICAL AND ON THE Ekey AND 43(hex)
		cmp edx, 0x00				// CHECKS TO SEE IF THE Ekey IS EQUAL TO 0(hex)
		jnz x16						// IF THE Ekey IS EQUAL TO 0, THEN JUMP TO X16
		mov edx, 0x07				// ELSE, SET THE Ekey TO 7(HEX)
		x16:       inc dl			// INCREMENTS THE LEAST SIGNIFICANT BYTE OF THE Ekey
		mov dword ptr[eax], edx		// MOVES A 32 BIT VALUE 
		pop ebx						// POP THE CHARACTER FROM THE INITIAL ecx REGISTER SAVED ON THE STACK AND MOVE THAT INTO ebx
		y16 : dec ebx				// DECREMENT ebx REGISTER BY 1 
		dec edx						// DECREMENT THE Ekey BY 1
		jnz y16						// IF THE Ekey IS EQUAL TO 0, THEN JUMP TO y16
		not bl						// INVERT THE 8 LEAST SIGNIFICANT BITS OF THE CHARACTER
		pop edx						// RESTORE THE ORIGINAL VALUE OF edx THAT WAS SAVED ON THE STACK
		mov eax, ebx				// POPULATES THE ORIGINAL Ekey (EAX) WITH THE NEW Ekey (EBX)
		pop ebx						// RESTORE THE ORIGINAL VALUE OF ebx THAT WAS SAVED ON THE STACK

		pop ebp						// RESTORE THE ORIGINAL VALUE OF ebp THAT WAS SAVED ON THE STACK
		ret;						// RETURN TO WHERE FUNCTION WAS CALLED
	}

	//--- End of Assembly code
}
//*** end of encrypt_chars function
//---------------------------------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------------------------------
//----------------- DECRYPTION ROUTINES -------------------------------------------------------------------------
//
void decrypt_chars(int length, char EKey)
{
		char temp_char;                     // Character temporary store

	for (int i = 0; i < length; i++)		// Decrypt characters one at a time
	{
		temp_char = EChars[i];				// Get the next char from Encrypted Chars array

		__asm {
			push   eax						// MOVE eax TO THE TOP OF THE STACK 
			push   ecx						// MOVE ecx TO THE TOP OF THE STACK
			movzx  ecx, temp_char			// MOVE THE ENCRYPTED CHARACTER INTO THE ecx REGISTER
			lea    eax, EKey				// MOVE THE Ekey INTO THE eax REGISTER
			push ecx						// MOVE ecx TO THE TOP OF THE STACK (CDECL)
			push eax						// MOVE eax TO THE TOP OF THE STACK (CDECL)
			call   decrypt_16				// CALLS decrypt_16 TO DECRYPT THE CURRENT CHAR
			mov    temp_char, al            // MOVE THE LEAST SIGNIFICANT BYTE OF THE eax REGISTER INTO temp_char
			
			cmp temp_char, 41h				// CHECK IF THE CHARACTER HEX CODE IS LESS THAN 41h WHICH IS EQUAL TO 'A'
			jl skipConversion				// IF IT IS, THEN SKIP THE CONVERSION PROCESS
			cmp temp_char, 5Ah				// CHECK IF THE CHARACTER HEX CODE IS LESS THAN 5Ah WHICH IS EQUAL TO 'Z'
			jg skipConversion				// IF IT IS, THEN SKIP THE CONVERSION PROCESS
			add temp_char, 20h				// IF IT REACHES THIS STAGE, THEN IT IS A CAPITAL LETTER BETWEEN A AND Z.
												//SO IT ADDS 20 TO THE HEX VALUE OF THE CHAR TO CONVERT IT TO LOWER CASE
			skipConversion :				// USED TO SKIP PAST THE CONVERSION IF THE CHARACTER ISN'T A CAPITAL LETTER
			
			add esp, 8						// ADDS 8 TO THE STACK POINTER IN ORDER TO REMOVE IT FROM THE STACK (CDECL)
			pop    ecx						// RESTORE THE ORIGINAL VALUE OF ECX (CDECL)
			pop    eax						// RESTORE THE ORIGINAL VALUE OF EAX (CDECL)
		}
		DChars[i] = temp_char;				// Store encrypted char in the Encrypted Chars array
	}
	return;

	__asm {

	decrypt_16:

		push ebp					// MOVES THE BASE POINTER TO THE TOP OF THE STACK (CDECL)
		mov ebp, esp				// MOVE THE STACK POINTER INTO THE BASE POINTER (CDECL)

		mov ecx, [ebp + 12]			// ADDS 12 (FROM THE BASE POINTER) TO ecx IN ORDER TO REMOVE IT FROM THE STACK (CDECL)
		mov eax, [ebp + 8]			// ADDS 8 (FROM THE BASE POINTER) TO eax IN ORDER TO REMOVE IT FROM THE STACK (CDECL)

		push ebx					// MOVE ebx REGISTER TO THE TOP OF THE STACK
		push edx					// MOVE edx REGISTER TO THE TOP OF THE STACK
		push ecx					// MOVE Encrypted Char TO THE TOP OF THE STACK
		movzx edx, byte ptr[eax]	// MOVE 8 LEAST SIGNIFICANT BITS FROM THE Ekey INTO THE EDX REGISTER
		and edx, 0x43				// PERFORMS A LOGICAL AND ON THE Ekey AND 43(hex)
		cmp edx, 0x00				// CHECKS TO SEE IF THE Ekey IS EQUAL TO 0(hex)
		jnz x16						// IF THE Ekey IS EQUAL TO 0, THEN JUMP TO X16
		mov edx, 0x07				// ELSE, SET THE Ekey TO 7(HEX)
		x16:       inc dl			// INCREMENTS THE LEAST SIGNIFICANT BYTE OF THE Ekey
		mov dword ptr[eax], edx		// MOVES A 32 BIT VALUE 
		pop ebx						// RESTORE THE ORIGINAL VALUE OF ebx THAT WAS SAVED ON THE STACK
		y16 : dec ebx				// DECREMENT ebx REGISTER BY 1 
		dec edx						// DECREMENT THE Ekey BY 1
		jnz y16						// IF THE Ekey IS EQUAL TO 0, THEN JUMP TO y16
		not bl						// INVERT THE 8 LEAST SIGNIFICANT BITS OF THE CHARACTER
		pop edx						// RESTORE THE ORIGINAL VALUE OF edx THAT WAS SAVED ON THE STACK
		mov eax, ebx				// POPULATES THE ORIGINAL Ekey (EAX) WITH THE NEW Ekey (EBX)
		pop ebx						// RESTORE THE ORIGINAL VALUE OF ebx THAT WAS SAVED ON THE STACK
		
		pop ebp						// RESTORE THE ORIGINAL VALUE OF ebp THAT WAS SAVED ON THE STACK (CDECL)
		ret							// RETURN TO WHERE THE FUNCTION WAS CALLED
	}
}

//*** end of decrypt_chars function
//---------------------------------------------------------------------------------------------------------------

int main(void)
{
	int char_count(0);  // The number of actual characters entered (upto MAXCHARS limit).

	cout << "\nPlease enter upto " << MAXCHARS << " alphanumeric characters:  ";
	get_original_chars(char_count);

	ofstream EDump;
	EDump.open("EncryptDump.txt", ios::app);
	EDump << "\n\nFoCA Encryption program results (" << StudentName << ") Encryption key = '" << EKey << "'";
	EDump << "\nDate: " << GetDate() << "  Time: " << GetTime();

	// Display and save initial string
	cout << "\n\nOriginal string =  " << OChars << "\tHex = ";
	EDump << "\n\nOriginal string =  " << OChars << "\tHex = ";
	for (int i = 0; i < char_count; i++)
	{
		cout << hex << setw(2) << setfill('0') << ((int(OChars[i])) & 0xFF) << "  ";
		EDump << hex << setw(2) << setfill('0') << ((int(OChars[i])) & 0xFF) << "  ";
	};

	//*****************************************************
	// Encrypt the string and display/save the result
	encrypt_chars(char_count, EKey);

	cout << "\n\nEncrypted string = " << EChars << "\tHex = ";
	EDump << "\n\nEncrypted string = " << EChars << "\tHex = ";
	for (int i = 0; i < char_count; i++)
	{
		cout << ((int(EChars[i])) & 0xFF) << "  ";
		EDump << ((int(EChars[i])) & 0xFF) << "  ";
	}

	//*****************************************************
	// Decrypt the encrypted string and display/save the result
	decrypt_chars(char_count, EKey);

	cout << "\n\nDecrypted string = " << DChars << "\tHex = ";
	EDump << "\n\nDecrypted string = " << DChars << "\tHex = ";
	for (int i = 0; i < char_count; i++)
	{
		cout << ((int(DChars[i])) & 0xFF) << "  ";
		EDump << ((int(DChars[i])) & 0xFF) << "  ";
	}
	//*****************************************************

	cout << "\n\n\n";
	EDump << "\n\n-------------------------------------------------------------";
	EDump.close();
	system("PAUSE");
	return (0);


} // end of whole encryption/decryption program --------------------------------------------------------------------
