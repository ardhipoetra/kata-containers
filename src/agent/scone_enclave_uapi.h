#ifndef _SCONE_ENCLAVE_UAPI_H
#define _SCONE_ENCLAVE_UAPI_H

#ifndef __KERNEL__
#include <stdint.h>
#else
#define uint8_t uint8_t
#endif

// _IO* from kernel persepective?
#define SCONE_IOC_PROVISION_KEY     _IOR ('a', 1, struct scone_provision_key)
#define SCONE_IOC_ENCLAVE_CREATE    _IOR ('a', 2, struct scone_enclave_create)
#define SCONE_IOC_ENCLAVE_ADD_PAGES _IOR ('a', 3, struct scone_enclave_add_pages)
#define SCONE_IOC_ENCLAVE_INIT      _IOR ('a', 4, struct scone_enclave_init)
#define SCONE_IOC_GET_SIGNED_QUOTE  _IOWR('a', 5, struct scone_enclave_get_quote)
#define SCONE_IOC_CERT              _IOWR('a', 11, scone_cert_t)
#define SCONE_IOC_DICE_INIT         _IOWR('a', 12, struct scone_dice_init)
#define SCONE_IOC_DICE              _IOR ('a', 13, struct scone_dice)

#define ED25519_KEY_SIZE 32

typedef struct scone_cert_body_s {
    uint8_t author_pubkey[32];
    uint8_t subject_pubkey[32];
    uint8_t measurement[32];
} scone_cert_body_t;

typedef struct scone_cert_s {
    scone_cert_body_t *body;
    uint8_t* cert_signature;
} scone_cert_t;

struct scone_dice {
    uint8_t* cdi;
    scone_cert_t out;
};

struct scone_dice_init {
    uint8_t uds[32];
    scone_cert_t out;
};

enum scone_page_flags {
    SCONE_PAGE_MEASURE    = 0x01,
};

enum sgx_page_type {
    SCONE_PAGE_TYPE_SECS,
    SCONE_PAGE_TYPE_TCS,
    SCONE_PAGE_TYPE_REG,
    SCONE_PAGE_TYPE_VA,
    SCONE_PAGE_TYPE_TRIM,
};

enum sgx_secinfo_flags {
    SCONE_SECINFO_R                   = 1 << 0,
    SCONE_SECINFO_W                   = 1 << 1,
    SCONE_SECINFO_X                   = 1 << 2,
    SCONE_SECINFO_SECS                = (SCONE_PAGE_TYPE_SECS << 8),
    SCONE_SECINFO_TCS                 = (SCONE_PAGE_TYPE_TCS << 8),
    SCONE_SECINFO_REG                 = (SCONE_PAGE_TYPE_REG << 8),
    SCONE_SECINFO_VA                  = (SCONE_PAGE_TYPE_VA << 8),
    SCONE_SECINFO_TRIM                = (SCONE_PAGE_TYPE_TRIM << 8),
};

enum scone_quote_key_type {
    KEY_ED25519 = 0,
};

struct scone_provision_key {
    enum scone_quote_key_type key_type;
    void *key;
    uint64_t key_size;
};

typedef struct {
    uint8_t cpusvn[16];
    uint8_t miscselect[4];
    uint8_t reserved1[28];
    uint8_t attributes[16];
    uint8_t mrenclave[32];
    uint8_t reserved2[32];
    uint8_t mrsigner[32];
    uint8_t reserved3[32];
    uint8_t configid[64];
    uint16_t isvprodid;
    uint16_t isvsvn;
    uint8_t configsvn[2];
    uint8_t reserved4[42];
    uint8_t isvfamilyid[16];
    uint8_t reportdata[64];
} sgx_report_body_t;

typedef struct scone_quote_s {
    sgx_report_body_t report;
    uint8_t enclave_pubkey[32];
    uint8_t report_signature[64];
} scone_quote_t;

struct scone_enclave_get_quote {
    uint8_t *report_data; // report data to put into the report
    uint64_t report_data_size; // size of the report data
    uint8_t *signed_quote; // output buffer for the quote
    uint64_t signed_quote_size; // size of the output buffer
};

struct sgx_secinfo_k {
    uint64_t flags;
    uint8_t  reserved[56];
};

struct scone_enclave_add_pages {
    uint8_t *data;
    uint64_t data_size;
    struct sgx_secinfo_k *secinfo;
    uint64_t flags;
    uint64_t offset;
};

struct scone_enclave_create  {
    uint64_t secs;
};

struct scone_enclave_init  {
    uint64_t sigstruct;
};

#endif
