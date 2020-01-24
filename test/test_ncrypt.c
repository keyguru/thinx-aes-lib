#import "../src/AESLib.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <iomanip>
#include <sstream>
#include <string>
#include <cstdint>

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

std::string intToHexString(uint8_t intValue) {
    std::string hexStr;
    std::stringstream sstream;
    sstream << std::setfill ('0') << std::setw(2)
    << std::hex << (int)intValue;
    hexStr = sstream.str();
    sstream.clear();
    return hexStr;
}

AESLib aesLib;

#define INPUT_BUFFER_LIMIT (128 + 1) // designed for Arduino UNO, not stress-tested anymore (this works with readBuffer[129])

char cleartext[INPUT_BUFFER_LIMIT] = {0}; // THIS IS INPUT BUFFER (FOR TEXT)
char ciphertext[2*INPUT_BUFFER_LIMIT] = {0}; // THIS IS OUTPUT BUFFER (FOR BASE64-ENCODED ENCRYPTED DATA)
char readBuffer[33] = "Looks like key but it's not me.";

// AES Encryption Key (same as in node-js example); should be updated from THiNX firmware as per-device private... or generated by device and reported.
// AES key has 16 bytes and is private, provided per-device by API or generated by device and sent back? (better).
// 2B 7E 15 16 28 AE D2 A6 AB F7 15 88 09 CF 4F 3C
byte aes_key[] = { 0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6, 0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C };

// General initialization vector (same as in node-js example) (you must use your own IV's in production for full security!!!)
byte aes_iv[N_BLOCK] = { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }; // 2 bytes (16 bits)
byte enc_iv_to[N_BLOCK] = { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }; // 2 bytes (16 bits)
byte enc_iv_from[N_BLOCK] = { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }; // 2 bytes (16 bits)

// Generate IV (once)
void aes_init() {
  aesLib.gen_iv(aes_iv);
}

// must not be in production code, ever
uint16_t encrypt_to_ciphertext(char * msg, byte iv[]) {
  uint16_t msgLen = strlen(msg);
  errno_t er = memset_s( ciphertext, sizeof(ciphertext), 0, sizeof(ciphertext) );
  if (er) return 0;
  uint16_t cipherLength = aesLib.encrypt((byte*)msg, msgLen, ciphertext, aes_key, sizeof(aes_key), iv);
  return cipherLength;
}

uint16_t decrypt_to_cleartext(byte msg[], uint16_t msgLen, byte iv[]) {
  errno_t er = memset_s( cleartext, sizeof(cleartext), 0, INPUT_BUFFER_LIMIT );
  if (er) return 0;
  uint16_t dec_len = aesLib.decrypt(msg, msgLen, cleartext, aes_key, sizeof(aes_key), iv);
  return dec_len;
}

// must not be in production code, ever
void test_ncrypt_1() {

    sprintf(cleartext, "%s", readBuffer); // copy from buffer because cleartext will be destroyed

    memcpy(aes_iv, enc_iv_to, sizeof(enc_iv_to));
    uint16_t len = encrypt_to_ciphertext(cleartext, aes_iv);

    errno_t er = memset_s( cleartext, sizeof(cleartext), 0, INPUT_BUFFER_LIMIT );
    if (er) return;

    memcpy(aes_iv, enc_iv_to, sizeof(enc_iv_from));
    uint16_t dec_len = decrypt_to_cleartext((byte*)ciphertext, len, aes_iv);

    bool mismatch = false;
    for (uint8_t pos = 0; pos < strlen(readBuffer); pos++) {
        if (readBuffer[pos] != cleartext[pos]) {
            printf("Mismatch found at %u\n", pos);
            mismatch = true;
        }
    }

    if (mismatch == false) {
        printf("\nTest passed. Decrypted cleartext is same as source read buffer.\n\n");
    }
}

void test_ncrypt_2() {
    test_ncrypt_1();
}

void test_ncrypt_3() {
    aesLib.getrnd();
    aesLib.gen_iv(aes_iv);
}


int main(int argc, char *argv[])
{
    printf("\n/*\n * THiNX AESLib Test\n */\n\n");

    printf("\nTest 1 – Pass 1...\n");
    test_ncrypt_1();

    printf("\nTest 1 – Pass 2...\n");
    test_ncrypt_2();

    printf("\nTest 2 - Pass 1+2...\n");
    test_ncrypt_3();
}
