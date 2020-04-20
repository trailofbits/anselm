#include <openssl/conf.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <string.h>

const unsigned char iv[16];

void f1(unsigned char *key) {
	EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
	if (!EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv)) return;
	EVP_CIPHER_CTX_free(ctx);
}

void f2(unsigned char *key) {
	EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
	if (!EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv)) return;
	EVP_CIPHER_CTX_free(NULL);
}

int f3(unsigned char *key, unsigned char *iv, unsigned char *plaintext, unsigned char *ciphertext, int flip) {
	int head, tail;
	EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
	if (!flip) {
		if (!EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv)) return -1;
	}
	if (!EVP_EncryptUpdate(ctx, ciphertext, &head, plaintext, strlen((char*) plaintext))) return -1;
	if (!EVP_EncryptFinal_ex(ctx, ciphertext+head, &tail)) return -1;
	EVP_CIPHER_CTX_free(ctx);
	return head + tail;
}

void f4(unsigned char *key, unsigned char *iv, unsigned char *plaintext, unsigned char *ciphertext, int n) {
	EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
	for (int i = 0; i < n; i++) {
		EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv);
	}
	EVP_CIPHER_CTX_free(ctx);
}
