#ifndef VLC_8KDVD_CERTIFICATE_VALIDATOR_H
#define VLC_8KDVD_CERTIFICATE_VALIDATOR_H

#include <vlc_common.h>
#include <vlc_input.h>
#include <vlc_fourcc.h>
#include <vlc_block.h>
#include <vlc_es.h>
#include <vlc_es_out.h>
#include <stdint.h>
#include <stdbool.h>

// 8KDVD Certificate Validator for Disc Authentication
typedef struct kdvd_certificate_validator_t kdvd_certificate_validator_t;

// 8KDVD Certificate Information
typedef struct kdvd_certificate_info_t {
    char certificate_id[64];          // Certificate ID
    char issuer[128];                 // Certificate issuer
    char subject[128];                // Certificate subject
    uint64_t valid_from;             // Valid from timestamp
    uint64_t valid_to;               // Valid to timestamp
    uint32_t version;                // Certificate version
    uint32_t signature_algorithm;    // Signature algorithm
    char public_key[256];            // Public key
    char signature[512];             // Digital signature
    bool is_valid;                   // Certificate validity
    bool is_revoked;                 // Certificate revocation status
    char revocation_reason[128];     // Revocation reason
    uint64_t revocation_date;        // Revocation date
} kdvd_certificate_info_t;

// 8KDVD Certificate Validation Statistics
typedef struct kdvd_certificate_stats_t {
    uint64_t certificates_checked;    // Total certificates checked
    uint64_t valid_certificates;      // Valid certificates
    uint64_t invalid_certificates;    // Invalid certificates
    uint64_t revoked_certificates;    // Revoked certificates
    uint64_t validation_time_us;     // Total validation time in microseconds
    float average_validation_time;    // Average validation time per certificate
    uint32_t current_certificate_count; // Current certificate count
    uint32_t memory_usage_mb;         // Memory usage in MB
} kdvd_certificate_stats_t;

// 8KDVD Certificate Validator Functions
kdvd_certificate_validator_t* kdvd_certificate_validator_create(vlc_object_t *obj);
void kdvd_certificate_validator_destroy(kdvd_certificate_validator_t *validator);

// Certificate Validation
int kdvd_certificate_validator_validate_certificate(kdvd_certificate_validator_t *validator, const char *certificate_path);
int kdvd_certificate_validator_validate_disc_certificate(kdvd_certificate_validator_t *validator, const char *disc_path);
int kdvd_certificate_validator_validate_chain(kdvd_certificate_validator_t *validator, const char *certificate_chain_path);

// Certificate Information
kdvd_certificate_info_t kdvd_certificate_validator_get_certificate_info(kdvd_certificate_validator_t *validator, const char *certificate_path);
int kdvd_certificate_validator_get_certificate_count(kdvd_certificate_validator_t *validator);
kdvd_certificate_info_t kdvd_certificate_validator_get_certificate(kdvd_certificate_validator_t *validator, uint32_t index);

// 8KDVD Specific Functions
int kdvd_certificate_validator_validate_8kdvd_certificate(kdvd_certificate_validator_t *validator, const char *disc_path);
int kdvd_certificate_validator_check_disc_authenticity(kdvd_certificate_validator_t *validator, const char *disc_path);
int kdvd_certificate_validator_verify_disc_integrity(kdvd_certificate_validator_t *validator, const char *disc_path);

// Certificate Chain Management
int kdvd_certificate_validator_add_trusted_ca(kdvd_certificate_validator_t *validator, const char *ca_certificate_path);
int kdvd_certificate_validator_remove_trusted_ca(kdvd_certificate_validator_t *validator, const char *ca_certificate_path);
int kdvd_certificate_validator_clear_trusted_cas(kdvd_certificate_validator_t *validator);

// Revocation Checking
int kdvd_certificate_validator_check_revocation(kdvd_certificate_validator_t *validator, const char *certificate_path);
int kdvd_certificate_validator_update_crl(kdvd_certificate_validator_t *validator, const char *crl_url);
int kdvd_certificate_validator_check_ocsp(kdvd_certificate_validator_t *validator, const char *certificate_path, const char *ocsp_url);

// Performance and Statistics
kdvd_certificate_stats_t kdvd_certificate_validator_get_stats(kdvd_certificate_validator_t *validator);
int kdvd_certificate_validator_reset_stats(kdvd_certificate_validator_t *validator);
int kdvd_certificate_validator_set_performance_mode(kdvd_certificate_validator_t *validator, const char *mode);

// Memory Management
int kdvd_certificate_validator_allocate_buffers(kdvd_certificate_validator_t *validator);
int kdvd_certificate_validator_free_buffers(kdvd_certificate_validator_t *validator);
int kdvd_certificate_validator_get_memory_usage(kdvd_certificate_validator_t *validator, uint32_t *usage_mb);

// Error Handling
int kdvd_certificate_validator_get_last_error(kdvd_certificate_validator_t *validator, char *error_buffer, size_t buffer_size);
int kdvd_certificate_validator_clear_errors(kdvd_certificate_validator_t *validator);

// Debug and Logging
void kdvd_certificate_validator_set_debug(kdvd_certificate_validator_t *validator, bool enable);
void kdvd_certificate_validator_log_info(kdvd_certificate_validator_t *validator);
void kdvd_certificate_validator_log_stats(kdvd_certificate_validator_t *validator);

#endif // VLC_8KDVD_CERTIFICATE_VALIDATOR_H
