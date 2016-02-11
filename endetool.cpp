#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "base64.h"
#include "aes256.h"
#include "endetool.h"

EnDeTool::EnDeTool()
 : origintext(NULL),
   encrypttext(NULL),
   cryptcontext(NULL),
   isencoded(false)
{
    memset( encryptkey, 0, 32 );

    aes256_context* actx = new aes256_context;
    cryptcontext = (void*)actx;
}

EnDeTool::~EnDeTool()
{
    if ( origintext != NULL )
    {
        delete[] origintext;
        origintext = NULL;
    }

    if ( encrypttext != NULL )
    {
        delete[] encrypttext;
        encrypttext = NULL;
    }
}

void EnDeTool::reset()
{
    if ( encrypttext != NULL )
    {
        delete[] encrypttext;
        encrypttext = NULL;
    }

    if ( origintext != NULL )
    {
        delete[] origintext;
        origintext = NULL;
    }

    return;
}

int  EnDeTool::encodebinary( const char* src, unsigned srcsize, char* &out )
{
    if ( ( src == NULL ) || ( srcsize == 0 ) )
        return -1;

    aes256_context* actx = (aes256_context*)cryptcontext;
    aes256_init( actx, (unsigned char*)encryptkey );

    int tmpCiperLen  = srcsize;

    if ( ( tmpCiperLen > 16 ) && ( ( tmpCiperLen % 16 ) != 0 ) )
    {
        tmpCiperLen += 16 - ( tmpCiperLen % 16 );
    }
    else
    {
        // Let minimal 16 bytes
        tmpCiperLen = 16;
    }

    if ( out != NULL )
    {
        delete[] out;
        out = NULL;
    }

    out = new char[ tmpCiperLen + 1 ];
    memset( out, 0, tmpCiperLen + 1 );
    memcpy( out, src, srcsize );

    int encloop = tmpCiperLen / 16;
    if ( encloop == 0 )
    {
        encloop = 1;
    }

    for ( int cnt=0; cnt<encloop; cnt++ )
    {
        aes256_encrypt_ecb( actx, (unsigned char*)&out[ cnt * 16 ] );
    }

    aes256_done( actx );

    return srcsize;
}

int  EnDeTool::decodebinary( const char* src, unsigned srcsize, char* &out )
{
    if ( ( src == NULL ) || ( srcsize < 16 ) )
        return -1;

    aes256_context* actx = (aes256_context*)cryptcontext;
    aes256_init( actx, (unsigned char*)encryptkey );

    if ( out != NULL )
    {
        delete[] out;
        out = NULL;
    }

    out = new char[ srcsize ];
    memset( out, 0, srcsize );
    memcpy( out, src, srcsize );

    int decloop = srcsize / 16;
    if ( decloop == 0 )
    {
        decloop = 1;
    }

    for (int cnt=0; cnt<decloop; cnt++ )
    {
        aes256_decrypt_ecb( actx, (unsigned char*)&out[cnt * 16] );
    }

    aes256_done( actx );

    return srcsize;
}

void EnDeTool::text( const char* srctext )
{
    if ( srctext == NULL )
        return;

    if ( origintext != NULL )
    {
        delete[] origintext;
        origintext = NULL;

        isencoded = false;
    }

    int srclen = strlen( srctext ) + 1;
    origintext = new char[ srclen ];
    memset( origintext, 0, srclen );
    strcpy( origintext, srctext );

    isencoded = encode();
}

void EnDeTool::encodedtext( const char* srctext )
{
    if ( srctext == NULL )
        return;

    if ( encrypttext != NULL )
    {
        delete[] encrypttext;
        encrypttext = NULL;
    }

    int srclen = strlen( srctext ) + 1;
    encrypttext = new char[ srclen ];
    memset( encrypttext, 0, srclen );
    strcpy( encrypttext, srctext );

    decode();
}

void EnDeTool::cryptkey( const char* key )
{
    if ( key == NULL )
        return;

    int cpylen = strlen( key );
    if ( cpylen > 8 )
    {
        cpylen = 8;
    }

    memset( encryptkey, 0, 32 );
    strncpy( encryptkey, key, cpylen );

    if ( isencoded == true )
    {
        isencoded = false;

        if ( encrypttext != NULL )
        {
            delete[] encrypttext;
            encrypttext = NULL;
        }
    }
}

bool EnDeTool::encode()
{
    if ( strlen(encryptkey) == 0 )
        return false;

    if ( cryptcontext == NULL )
        return false;

    aes256_context* actx = (aes256_context*)cryptcontext;
    aes256_init( actx, (unsigned char*)encryptkey );

    int   srcLen       = strlen(origintext);
    int   tmpCiperLen  = srcLen;

    // AES-256 encodes 16 bytes in once.
    // Make buffer pads with 16 multiply.
    if ( ( tmpCiperLen > 16 ) && ( ( tmpCiperLen % 16 ) != 0 ) )
    {
        tmpCiperLen += 16 - ( tmpCiperLen % 16 );
    }
    else
    {
        // Let minimal 16 bytes
        tmpCiperLen = 16;
    }

    char* tmpCiperBuff = new char[ tmpCiperLen + 1 ];
    memset( tmpCiperBuff, 0, tmpCiperLen + 1 );
    memcpy( tmpCiperBuff, origintext, srcLen );

    int encloop = tmpCiperLen / 16;
    if ( encloop == 0 )
    {
        encloop = 1;
    }

    for ( int cnt=0; cnt<encloop; cnt++ )
    {
        aes256_encrypt_ecb( actx, (unsigned char*)&tmpCiperBuff[ cnt * 16 ] );
    }

    aes256_done( actx );

    int outBase64Len = tmpCiperLen * 2;

    char* tmpBase64Buff = NULL;

    int retI = base64_encode( tmpCiperBuff, tmpCiperLen, &tmpBase64Buff );

    delete[] tmpCiperBuff;
    tmpCiperBuff = NULL;

    if ( ( retI > 0 ) && ( tmpBase64Buff != NULL ) )
    {
        if ( encrypttext != NULL )
        {
            delete[] encrypttext;
            encrypttext = NULL;
        }

        encrypttext = new char[ retI + 1 ];
        if ( encrypttext != NULL )
        {
            memset( encrypttext, 0, retI + 1 );
            strncpy( encrypttext, tmpBase64Buff, retI );
        }

        free( tmpBase64Buff );
        tmpBase64Buff = NULL;

        return true;
    }

    return false;
}

bool EnDeTool::decode()
{
    if ( strlen(encryptkey) == 0 )
        return false;

    if ( cryptcontext == NULL )
        return false;

    if ( encrypttext == NULL )
        return false;

    aes256_context* actx = (aes256_context*)cryptcontext;
    aes256_init( actx, (unsigned char*)encryptkey );

    int tmpCiperLen  = strlen(encrypttext);
    if ( tmpCiperLen == 0 )
        return false;

    char* tmpCiperBuff = new char[ tmpCiperLen + 1 ];
    memset( tmpCiperBuff, 0, tmpCiperLen + 1 );
    int retI = base64_decode( encrypttext, (unsigned char*)tmpCiperBuff, tmpCiperLen );
    if ( retI == 0 )
        return false;

    if ( origintext != NULL )
    {
        delete[] origintext;
        origintext = NULL;
    }

    int decloop = tmpCiperLen / 16;
    if ( decloop == 0 )
    {
        decloop = 1;
    }

    for (int cnt=0; cnt<decloop; cnt++ )
    {
        aes256_decrypt_ecb( actx, (unsigned char*)&tmpCiperBuff[cnt * 16] );
    }

    // Let make it erase wrong buffers.
    int clearLen = tmpCiperLen - retI;

    memset( &tmpCiperBuff[retI], 0, clearLen );

    origintext = tmpCiperBuff;

    aes256_done( actx );

    return true;
}
