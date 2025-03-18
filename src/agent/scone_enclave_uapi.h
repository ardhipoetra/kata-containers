#ifndef _SCONE_ENCLAVE_UAPI_H
#define _SCONE_ENCLAVE_UAPI_H

#ifndef __KERNEL__
#include <stdint.h>
#else
#define uint8_t uint8_t
#endif

#define SCONE_IOC_HIEST_INIT         _IOWR('a', 12, struct scone_hiest_init)
#define SCONE_IOC_HIEST              _IOWR('a', 13, struct scone_hiest)
#define SCONE_IOC_HIEST_SELF         _IOR ('a', 14, struct scone_hiest_self)
#define SCONE_IOC_HIEST_SIGN         _IOWR('a', 15, struct scone_hiest_sign)
#define SCONE_IOC_HIEST_INIT_NEW     _IO  ('a', 16)
#define SCONE_IOC_HIEST_INIT_GET     _IOR('a', 17, struct scone_hiest_init_get)

#define ED25519_KEY_SIZE 32

typedef struct scone_cert_body_s {
    uint8_t author_pubkey[32];
    uint8_t subject_pubkey[32];
    uint8_t measurement[32];
    uint8_t cdi_hash[32];
    uint8_t prev_cert_hash[32];
} scone_cert_body_t;

typedef struct scone_cert_s {
    scone_cert_body_t *body;
    uint8_t* cert_signature;
} scone_cert_t;

struct scone_hiest_self {
    uint8_t* cdi;
    scone_cert_t out[2];
};

struct scone_hiest {
    uint8_t fd;
    uint8_t* cdi_in; // this one is not used anymore (CDI is a secret) - previously CDI of current layer
    uint8_t* cdi;   //  this too, historically this is the CDI of the *new* layer
    scone_cert_t* cert_in;
    scone_cert_t out;
};

struct scone_hiest_init {
    uint8_t uds[32]; // this one is not used anymore, you need to call SCONE_IOC_HIEST_INIT_NEW beforehand
    scone_cert_t out;
};

struct scone_hiest_sign {
    uint8_t fd;
    uint8_t* data;
    uint8_t data_size;
    uint8_t* cert_signature_in;
    
    uint8_t signature_out[64];
};

struct scone_hiest_init_get {
    uint8_t pubkey[32];
};

#endif
