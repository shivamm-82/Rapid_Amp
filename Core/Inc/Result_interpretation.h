#ifndef PCR_RESULT_H
#define PCR_RESULT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <math.h>

/* ─────────────────────────────────────────────
 * USER CONFIGURATION — adjust to your assay
 * ───────────────────────────────────────────── */
#define SIZE_OF_PHOTO_DATA_CAPTURE   30u    /* total readings per well        */
#define NUM_CHANNELS                  7u    /* number of wells                */
#define IC_CHANNEL_INDEX              6u    /* WELL_7 = Internal Control      */

#define BASELINE_POINTS               5u    /* first N readings for baseline  */
#define THRESHOLD_MULTIPLIER         10.0f  /* mean + N * SD                  */
#define MIN_THRESHOLD_DELTA         200.0f  /* absolute minimum rise (counts) */
#define CONSECUTIVE_REQUIRED          3u    /* N points above threshold → +ve */
#define TT_VALID_MIN                  4u    /* earliest valid Tt (minutes)    */
#define TT_VALID_MAX                 28u    /* latest  valid Tt (minutes)     */

/* ─────────────────────────────────────────────
 * DATA STRUCTURES
 * ───────────────────────────────────────────── */

/** Raw reading buffer — your existing struct, unchanged */
typedef struct {
    uint32_t Buff[SIZE_OF_PHOTO_DATA_CAPTURE];
} POSITION;

/** Result for one well */
typedef enum {
    RESULT_INVALID  = -1,
    RESULT_NEGATIVE =  0,
    RESULT_POSITIVE =  1
} WellResult;

typedef struct {
    int8_t      Tt;           /* minute of threshold crossing (-1=neg, -2=invalid window) */
    WellResult  result;       /* final result for this well                                */
    uint32_t    end_signal;   /* last raw ADC reading                                      */
    float       fold_rise;    /* end_signal / baseline_mean                                */
    bool        done;         /* true = result already called (early exit)                 */
} WELL_RESULT;




typedef enum {
    FINAL_DETECTED        =  1,   /* one or more targets positive       */
    FINAL_NOT_DETECTED    =  0,   /* all targets negative, IC valid     */
    FINAL_INVALID         = -1,   /* IC failed + all targets negative   */
    FINAL_INCONCLUSIVE    = -2,   /* some wells invalid, no positives   */
} FinalRunResult;

typedef struct {
    FinalRunResult  overall;
    uint8_t         positive_mask;   /* bit N set = WELL_(N+1) positive  */
    uint8_t         invalid_mask;    /* bit N set = WELL_(N+1) invalid   */
    uint8_t         positive_count;  /* how many targets positive        */
} FINAL_RESULT;


/* ─────────────────────────────────────────────
 * PUBLIC API
 * ───────────────────────────────────────────── */

/**
 * @brief  Initialize / reset all state for a new run.
 *         Call once before starting a new PCR run.
 * @param  well_ptrs  Array of 7 pointers to POSITION structs
 *                    e.g. {&WELL_1, &WELL_2, ... &WELL_7}
 */
void PCR_Init(POSITION *well_ptrs[NUM_CHANNELS]);

/**
 * @brief  Call every minute after storing the new reading into Buff[].
 *         Handles baseline locking and early positive detection.
 *         Automatically calls PCR_FinalResults() at minute 30.
 * @param  minute  Current reading index (0-based, i.e. minute 0..29)
 */
void PCR_OnMinuteTick(uint8_t minute);

/**
 * @brief  Force final result calculation on all wells.
 *         Called automatically by PCR_OnMinuteTick at the last reading.
 *         Can also be called manually if you want results mid-run.
 */
void PCR_FinalResults(void);

/**
 * @brief  Get result for a specific well (0-based index).
 * @param  ch  Channel index 0..6
 * @return Pointer to WELL_RESULT struct (read-only)
 */
const WELL_RESULT *PCR_GetResult(uint8_t ch);

/**
 * @brief  Get baseline mean for a channel (valid after minute 5).
 * @param  ch  Channel index 0..6
 */
float PCR_GetBaselineMean(uint8_t ch);

/**
 * @brief  Get computed threshold for a channel (valid after minute 5).
 * @param  ch  Channel index 0..6
 */
float PCR_GetThreshold(uint8_t ch);

/**
 * @brief  Returns true if all 7 wells have a final result (early exit possible).
 */
bool PCR_AllDone(void);




FINAL_RESULT PCR_GetFinalRunResult(void);

void Negative_led_indicate(void);

void Positive_led_indicate(void);

void Invalid_led_indicate(void);


#ifdef __cplusplus
}
#endif

#endif /* PCR_RESULT_H */
