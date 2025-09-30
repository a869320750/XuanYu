
#ifndef __SMS4_H__
#define __SMS4_H__

#ifdef __cplusplus
 extern "C" {
#endif 

#define SMS4_ROUND     32                                                    
#define ROL(x,y)       (((x) << (y)) | (((x) & 0xFFFFFFFFul) >> (32 - (y))))



unsigned int  SMS4_Decrypt(unsigned char * SMS4_src,unsigned char * SMS4_dest,unsigned char * SMS4_key);

unsigned int  SMS4_Encrypt(unsigned char * SMS4_src,unsigned char * SMS4_dest,unsigned char * SMS4_key);

unsigned int Aurh_SMS4_Function(unsigned char* SMS4_src,unsigned char* SMS4_dest);

unsigned  int DMT_SMS4_Encrypt_Ex(unsigned char * SMS4_dest,unsigned char * SMS4_src,unsigned char * SMS4_key,unsigned int SMS4_src_len);

unsigned int DMT_SMS4_Decrypt_Ex(unsigned char * SMS4_dest,unsigned char * SMS4_src,unsigned char * SMS4_key, unsigned int SMS4_src_len);

unsigned int DSM2Key_SMS4_Function(unsigned char* SMS4_src,unsigned char* SMS4_dest);

#ifdef __cplusplus
}
#endif

#endif
