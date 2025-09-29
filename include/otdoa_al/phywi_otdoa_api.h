/*------------------------------------------------------------------+
 *        ACORN TECHNOLOGIES PROPRIETARY AND CONFIDENTIAL           |
 *       Copyright (C)  PHY Wireless, LLC a wholly owned subsidiary |
 *             of Acorn Technologies, Inc.  2015-2025               |
 *                   All Rights Reserved                            |
 *------------------------------------------------------------------+
 *
 * phywi_otdoa_api.h - API Definitions for the PHY Wireless OTDOA module
 *
 */

#ifndef PHYWI_OTDOA_API__
#define PHYWI_OTDOA_API__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file phywi_otdoa_api.h
 *
 * @defgroup phywi_otdoa_api PHY Wireless OTDOA module
 * @{
 * @brief API Definitions for the PHY Wireless OTDOA module.
 */

#define OTDOA_API_MAX_CELLS     75
#define OTDOA_API_MAX_SESSION_LEN   250

#ifdef HOST
    #define LOG2_COMPRESS_WINDOW    15      // use large decomp. window on HOST so we can decompress files compressed with gzip
#else
    #define LOG2_COMPRESS_WINDOW    9       // Log2 of compression window size (i.e. 10=>1024B compression window)
#endif


/// @brief Maximum length of the uBSA file path 
#define OTDOA_API_MAX_UBSA_FILE_PATH    50  

typedef enum {
    OTDOA_API_SUCCESS     =  0,     // Success

    /** Return values from API function calls */
    OTDOA_API_ERROR_PARAM = -1,     // Parameter error in API
    OTDOA_API_INTERNAL_ERROR = -2,  // Internal error in OTDOA library. 


    /** EVENT_FAIL result code values */
    /** Failure occurred in the uBSA download */
    OTDOA_EVENT_FAIL_UBSA_DL = 1,
    
    /** Network failure occurred during OTDOA processing */
    OTDOA_EVENT_FAIL_NO_CELL = 2,

    /** PRS session cancelled by application */
    OTDOA_EVENT_FAIL_CANCELLED = 3,

    /** Other failure occurred in OTDOA processing */
    OTDOA_EVENT_FAIL_OTDOA_PROC = 4,
    
    /** Session timeout occurred during OTDOA processing or uBSA DL */
    OTDOA_EVENT_FAIL_TIMEOUT = 5,

    /** Error occurred in nrf modem RS capture API */
    OTDOA_EVENT_FAIL_NRF_RS_CAPTURE = 6,

    /** Error in response from Modem */
    OTDOA_EVENT_FAIL_BAD_MODEM_RESP = 7,

    /** Modem is not registered to LTE network */
    OTDOA_EVENT_FAIL_NOT_REGISTERED = 8,
    
    /** Modem has stopped, a higher prority activity stopped the session. */
    OTDOA_EVENT_FAIL_STOPPED = 9,
    
    /** Failed to get DLEARFCN from Modem */
    OTDOA_EVENT_FAIL_NO_DLEARFCN = 10,

    /** System Mode is not LTE (e.g. it may be NBIoT mode) */
    OTDOA_EVENT_FAIL_NOT_LTE_MODE = 11,

    /** Failure to parse the uBSA file */
    OTDOA_EVENT_FAIL_UBSA_PARSING = 12,

    /** Bad Config file */
    OTDOA_EVENT_FAIL_BAD_CFG = 13,

} otdoa_api_error_codes_t; 

/** @brief OTDOA Session Parameters 
 * These parameters are sent to the OTDOA library as part of
 * the @ref otdoa_api_start_session() API call.
 */
typedef struct {
    /** session length in PRS occasions */
    uint32_t session_length;    

    /** Test/Debug flags */
    uint32_t capture_flags;

    /** session timeout in msec. */
    uint32_t timeout;

} otdoa_api_session_params_t;


/** 
 * @brief Details of the OTDOA results 
 * This structure is returned along with the position
 * estimate as a part of the OTDOA_EVENT_RESULTS event.
 */
typedef struct {
    
    /** ECGI of the serving cell during the OTDOA processing*/
    uint32_t serving_cell_ecgi;

    /** RSSI of the serving cell, in dBm */
    int32_t serving_rssi_dbm;
    
    /** DLEARFCN of serving cell */
    uint32_t dlearfcn;

    /** Text description of the algorithm used (OTDOA, eCID)*/
    char estimate_algorithm[30];
    
    /** The length of the PRS session in PRS occasions */
    uint32_t session_length;

    /** Number of cells that we measured in the OTDOA process */
    uint32_t num_measured_cells;

    /** Array containing the ECGIs of the measured cells
        only the first num_measured_cells entries in this array are valid */
    uint32_t ecgi_list[OTDOA_API_MAX_CELLS];

    /** Array containing the number of times each cell has its TOA successfully detected
        only the first num_measured_cells entries in this array are valid */
    uint16_t toa_detect_count[OTDOA_API_MAX_CELLS];

} otdoa_api_result_details_t;


/**
 * @brief OTDOA Results
 * This structure contains the results of the OTDOA position estimation
 * It is returned as a part of the OTDOA_EVENT_RESULTS event.
 */
typedef struct {
	/** Geodetic latitude (deg) in WGS-84. */
	double latitude;
	/** Geodetic longitude (deg) in WGS-84. */
	double longitude;

	/** Location accuracy estimate in ./otdoa_lib/src/top/meters. */
	float accuracy;

    /** Details of the OTDOA estimated */
    otdoa_api_result_details_t details;
} otdoa_api_results_t;


/** 
 * @brief Parameters of the uBSA download request. 
 * This structure is sent along with the @ref OTDOA_EVENT_UBSA_DL_REQ event.
 * It is also returned to the OTDOA library in the @ref otdoa_api_ubsa_download()
 * API call.
*/
typedef struct {
	/** PLMN ID.
	 *
	 * Bytes encoded as follows:
	 *     mcc2_mcc1, mnc3_mcc3, mnc2_mnc1
	 */
	uint8_t plmn[3];
	/** 28bit cell id. */
	uint32_t ecgi;
	/** Mobile Country Code. Range: 0...999. */
	uint16_t mcc;
	/** Mobile Network Code. Range: 0...999. */
	uint16_t mnc;

    /** Cell DLEARFCN */
    uint32_t dlearfcn;

    /** Maximum number of cells in the uBSA */
    uint32_t max_cells;

    /** Maximum uBSA radius in meters */
    uint32_t ubsa_radius_meters;
    

} otdoa_api_ubsa_dl_req_t;

/** 
 * @brief Parameters of the uBSA download complete event
 * This structure is returned by the OTDOA library along
 * with the OTDOA_EVENT_UBSA_DL_COMPL event to indicate that
 * the download of the uBSA has completed
 */
enum  otdoa_api_ubsa_dl_compl_status {   //Added name otdoa_api_ubsa_dl_compl_status
    OTDOA_DL_STATUS_SUCCESS = 0,
    
    /** A failure occurred in connection to the network */
    OTDOA_DL_STATUS_FAIL_NTWK_CONN,
    
    /** An error was reported by the server */
    OTDOA_DL_STATUS_SERVER_ERROR,
    
    /** The download process was interrupted or cancelled */
    OTDOA_DL_STATUS_CANCELLED,
    
    /** A problem was detected in the donwloaded uBSA file */
    OTDOA_DL_STATUS_BAD_FILE,
    
    /** Problem in download of config file */
    OTDOA_DL_STATUS_BAD_CFG,
    
    /** Problem in download request */
    OTDOA_DL_STATUS_BAD_REQ,
    
    /** Problem with authentication of download request */
    OTDOA_DL_STATUS_AUTH_FAIL,
    
    /** Problem report by server, UE may retry */
    OTDOA_DL_STATUS_SERVER_ERROR_RETRY_OK,

    /** Server indicated an error where the UE should not retry */
    OTDOA_DL_STATUS_SERVER_ERROR_NO_RETRY,

    /** Unknown or unclassified error occurred */
    OTDOA_DL_STATUS_OTHER_ERROR,
    
};

typedef struct {
    /** Status of the uBSA download */
    enum otdoa_api_ubsa_dl_compl_status status; 
} otdoa_api_ubsa_dl_compl_t;

typedef struct {
    /** Status of the results upload*/
    enum otdoa_api_ubsa_dl_compl_status status;    // The same status values are used for UL of results and DL of uBSA
} otdoa_api_ubsa_ul_compl_t;

/** @brief enum defining the type of event retured by the OTDOA library */
enum otdoa_api_event_id {
    /** Position estimate results from OTDOA */
    OTDOA_EVENT_RESULTS = 1,

    /** OTDOA library requests download of a uBSA */
    OTDOA_EVENT_UBSA_DL_REQ,
    
    /** OTDOA library indicates download of uBSA is complete */
    OTDOA_EVENT_UBSA_DL_COMPL,

    /** OTDOA Library indicates completion of results upload */
    OTDOA_EVENT_RESULTS_UL_COMPL,

    /** Failure */
    OTDOA_EVENT_FAIL
};

typedef struct {
    enum otdoa_api_event_id event;

    /** Event-specific data */
    union {
        /** OTDOA_EVENT_RESULTS event details */
        otdoa_api_results_t results;

        /** OTDOA_EVENT_UBSA_DL_REQ event details */
        otdoa_api_ubsa_dl_req_t dl_request;
        
        /* OTDOA_EVENT_UBSA_DL_COMPL event details */
        otdoa_api_ubsa_dl_compl_t dl_compl;

        /* OTDOA_EVENT_UBSA_UL_COMPL event details */
        otdoa_api_ubsa_ul_compl_t ul_compl;

        /** OTDOA_EVENT_FAIL event details*/
        int32_t failure_code;

    };
} otdoa_api_event_data_t;

/** 
 * @brief OTDOA event callback prototype
 * 
 * @param[in] event_data Event Data
 */
typedef void (*otdoa_api_callback_t)(
    const otdoa_api_event_data_t* event_data);


/**
 * @brief Initialize the OTDOA library
 * @param[in] ubsa_file_path Points to a string containing the full path to where
 *                           the uBSA file resides
 * @param[in] callback Callback function used by the library
 *                     to return results and status to the client
 * @return 0 on success
 */
int32_t otdoa_api_init(const char* const ubsa_file_path, otdoa_api_callback_t event_callback);

/**
 * @brief Initiates an OTDOA positioning session
 * @param[in] params Parameters of the session, including
 *                   session length, timeout, etc.
 * @param[in] callback Callback function used by the library
 *                     to return results and status to the client
 * @retval 0 on success
 */
int32_t otdoa_api_start_session(
    const otdoa_api_session_params_t* params,
    otdoa_api_callback_t callback
);

/**
 * @brief Cancels an on-going OTDOA session
 * @return 0 on success
 */
int32_t otdoa_api_cancel_session();

/**
 * @brief Requests that the OTDOA library initiate download of a new uBSA file.
 * @param[in] dl_request Structure containing the parameters of the requested uBSA
 * @param[in] ubsa_file_path Points to a string containing the full path to where
 *                           the uBSA file should be written
 * @retval Error codes as defined in otdoa_api_error_codes_t
 */
int32_t otdoa_api_ubsa_download(const otdoa_api_ubsa_dl_req_t* dl_request,
                                const char* const ubsa_file_path);

/**
 * @brief Requests that the OTDOA library initiate download of a new configuration file.
 *
 * @retval Error codes as defined in otdoa_api_error_codes_t
 */
int otdoa_api_cfg_download(void);

/*
 * @brief Indicates that an updated uBSA is available to the OTDOA library
 * @param[in] status A non-zero value indicates failure to update the uBSA. 
 * @param ubsa_file_path Pointer to a null-terminated string containing the
 *                       full path to the updated uBSA file.
 * @retval Error codes as defined in otdoa_api_error_codes_t
 */
int32_t otdoa_api_ubsa_available(
    int32_t status,
    const char* const ubsa_file_path);


/**
 * @brief Upload the OTDOA results to the PhyWi Server
 * @param[in] p_results Points to a structure containing the
 *                      postion estimate results
 * @param true_lat[in]  Points to a string containing the "ground truth" latitude
 * @param true_lon[in]  Points to a string containing the "ground truth" longitude
 * @param notes[in]     Points to a string containing notes included with the upload
 * 
 * @retval Error codes as defined in otdoa_api_error_codes_t
 * 
 * @note Any of true_lat, true_lon and notes may be NULL.
 */
#define OTDOA_NOTES_MAX 81  // Maximum size of the notes string uploaded with results

int32_t otdoa_api_upload_results(const otdoa_api_results_t* p_results,
                                 const char* true_lat,  const char* true_lon,
                                 const char* notes);

/**
 * @brief Provisions a key for use with the OTDOA library
 * 
 * @retval 0 on success
 *        -1 on failure
 */
int otdoa_api_provision(const char* key_string);

/**
 * @brief Returns a constant string containing the OTDOA library version
 */
const char* const otdoa_api_get_version();

/**
 * @brief Sets the path used for the config file
 */
void otdoa_api_cfg_set_file_path(const char * psz_path);

/**
 * @brief Gets the path used for the config file
 */
const char* otdoa_api_cfg_get_file_path();

/** @} */

#ifdef __cplusplus
}
#endif

#endif  // PHYWI_OTDOA_API__

