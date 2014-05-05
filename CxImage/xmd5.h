/* crypto/md5/md5.h */
/* Copyright (C) 1995-1998 Eric Young (eay@cryptsoft.com)
 * All rights reserved.
 *
 * This package is an SSL implementation written
 * by Eric Young (eay@cryptsoft.com).
 * The implementation was written so as to conform with Netscapes SSL.
 * 
 * This library is free for commercial and non-commercial use as long as
 * the following conditions are aheared to.  The following conditions
 * apply to all code found in this distribution, be it the RC4, RSA,
 * lhash, DES, etc., code; not just the SSL code.  The SSL documentation
 * included with this distribution is covered by the same copyright terms
 * except that the holder is Tim Hudson (tjh@cryptsoft.com).
 * 
 * Copyright remains Eric Young's, and as such any Copyright notices in
 * the code are not to be removed.
 * If this package is used in a product, Eric Young should be given attribution
 * as the author of the parts of the library used.
 * This can be in the form of a textual message at program startup or
 * in documentation (online or textual) provided with the package.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *    "This product includes cryptographic software written by
 *     Eric Young (eay@cryptsoft.com)"
 *    The word 'cryptographic' can be left out if the rouines from the library
 *    being used are not cryptographic related :-).
 * 4. If you include any Windows specific code (or a derivative thereof) from 
 *    the apps directory (application code) you must include an acknowledgement:
 *    "This product includes software written by Tim Hudson (tjh@cryptsoft.com)"
 * 
 * THIS SOFTWARE IS PROVIDED BY ERIC YOUNG ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 * 
 * The licence and distribution terms for any publically available version or
 * derivative of this code cannot be changed.  i.e. this code cannot simply be
 * copied and put under another distribution licence
 * [including the GNU Public Licence.]
 */

#ifndef HEADER_MD5_H
#define HEADER_MD5_H

#include <string.h>   

#ifdef  __cplusplus
extern "C" {
#endif


  
#define MD5_DIGEST_SIZE     16   
#define MD5_HMAC_BLOCK_SIZE     64   
#define MD5_BLOCK_WORDS     16   
#define MD5_HASH_WORDS      4   
  
typedef unsigned char       u8;  
typedef unsigned short      u16;  
typedef unsigned int        u32;  
typedef unsigned long int   u64;  
  
typedef struct md5_state {  
    u32 hash[MD5_HASH_WORDS];  
    u32 block[MD5_BLOCK_WORDS];  
    u64 byte_count;  
}XMD5_CTX;  
  
/* 
    名称: md5_init 
    功能: 初始化MD5 
    参数:  
            输入: 
                1:struct md5_state *desc;   MD5结构体 
    返回值: 
            return 0; 
*/  
int   
md5_init(XMD5_CTX *desc);  
  
/* 
    名称: md5_update 
    功能: 初始化MD5 
    参数:  
            输入: 
                1:struct md5_state *desc    MD5结构体 
                2:const u8 *data        数据指针 
                3:unsigned int len      数据长度 
    返回值: 
            return 0; 
*/  
int   
md5_update( XMD5_CTX *desc, const u8 *data, unsigned int len);  
  
/* 
    名称: md5_final 
    功能: 获取结果 
    参数:  
            输入: 
                1:struct md5_state *desc    MD5结构体 
            输出: 
                2:u8 *out           结果.长度为16    eg.u8 out[16]; 
    返回值: 
            return 0; 
*/  
int   
md5_final( XMD5_CTX *desc, u8 *out);  
  
/* 
    名称: md5_cale 
    功能: 计算data指向的数据的MD5值,直接返回在out数组内 
    参数:  
            输入: 
                1:const u8 *data            数据指针 
                2:unsigned int len          数据长度 
            输出: 
                3:u8 *out               结果.长度为16    eg.u8 out[16]; 
    返回值: 
            return 0; 
*/  
int   
md5_cale(const u8 *data, unsigned int len,u8 *out);  

#ifdef  __cplusplus
}
#endif

#endif
