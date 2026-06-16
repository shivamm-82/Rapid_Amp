/*
 * Result_interpretation.c
 *
 *  Created on: May 22, 2026
 *      Author: Shivam.Maurya
 */


#include "Result_interpretation.h"
#include "Basic_functions_1.h"
#include <stdint.h>
#include <math.h>
/**
 * @file    pcr_result.c
 * @brief   Isothermal PCR result-calling algorithm
 *          - 7 wells, 30 readings (1 per minute)
 *          - Dynamic threshold: mean + N*SD of baseline
 *          - Early positive detection per minute tick
 *          - Full negative/invalid call at minute 30
 *          - IC (Internal Control) gating on WELL_7
 *
 * @note    Adapt SIZE_OF_PHOTO_DATA_CAPTURE, THRESHOLD_MULTIPLIER,
 *          TT_VALID_MIN/MAX to your assay in pcr_result.h
 */

#include <string.h>   /* memset */

#include "main.h"

extern uint8_t SOMETHING_DETECTED;
extern void Buzzer_Test_Complete_tone(void);    //from main.c
extern void send_usb_string(const char* str);  //from main.c
extern void send_BLE_using_USART3_string(const char* str); //from main.c
/* ─────────────────────────────────────────────
 * PRIVATE STATE
 * ───────────────────────────────────────────── */

static POSITION    *s_wells[NUM_CHANNELS];           /* pointers to user's POSITION structs */
static WELL_RESULT  s_results[NUM_CHANNELS];
static float        s_baseline_mean[NUM_CHANNELS];
static float        s_baseline_sd[NUM_CHANNELS];
static float        s_threshold[NUM_CHANNELS];
static bool         s_baseline_locked = false;

/* ─────────────────────────────────────────────
 * PRIVATE HELPERS
 * ───────────────────────────────────────────── */

/**
 * @brief  Compute baseline mean + SD from first BASELINE_POINTS readings.
 *         Threshold is also computed and cached here.
 *         Called once at minute BASELINE_POINTS and never again.
 */

FINAL_RESULT PCR_GetFinalRunResult(void)
{
    FINAL_RESULT final = {0};

    bool ic_passed     = (s_results[3].result == RESULT_POSITIVE);  //at well 4 ic assumed
    bool any_positive  = false;
    bool any_invalid   = false;

    /* Check each target well (WELL_1 to WELL_6) */
    for (uint8_t ch = 0; ch < IC_CHANNEL_INDEX; ch++)
    {
        if (s_results[ch].result == RESULT_POSITIVE) {
            any_positive = true;
            final.positive_count++;
            final.positive_mask |= (1 << ch);   /* bit 0 = WELL_1, bit 5 = WELL_6 */
        }
        else if (s_results[ch].result == RESULT_INVALID) {
            any_invalid = true;
            final.invalid_mask |= (1 << ch);
        }
    }

    /* ── Decision tree ── */
    if (any_positive)
    {
        /* At least one target found — report DETECTED regardless of IC */
    	if(ic_passed){
        final.overall = FINAL_DETECTED;
        Positive_led_indicate();

        //send to PC
        send_usb_string("{0x07}\n"); //positive
        HAL_Delay(20);
        send_usb_string("{0x0A}\n"); //positive detected
        HAL_Delay(20);

        //send to BLE
         send_BLE_using_USART3_string("{0x07}\n"); //positive
         HAL_Delay(20);
         send_BLE_using_USART3_string("{0x0A}\n"); //positive detected
         HAL_Delay(20);

        Buzzer_Test_Complete_tone();

    	}
    }
    else if (!ic_passed)
    {
        /* IC failed + no positives = reaction failure */
        final.overall = FINAL_INVALID;
        Invalid_led_indicate();

        //send to PC
        send_usb_string("{0x09}\n"); //invalid
        HAL_Delay(20);
        send_usb_string("{0x0B}\n"); //IC not detected
        HAL_Delay(20);

        //Send to BLE
        send_BLE_using_USART3_string("{0x09}\n"); //invalid
        HAL_Delay(20);
        send_BLE_using_USART3_string("{0x0B}\n"); //IC not detected
        HAL_Delay(20);

        Buzzer_Test_Complete_tone();
    }
//    else if (any_invalid)
//    {
//        /* IC passed, no positives, but some wells had issues */
//        final.overall = FINAL_INCONCLUSIVE;
//        Invalid_led_indicate();
//        Buzzer_Test_Complete_tone();
//    }
    else
    {
        /* IC passed, all targets cleanly negative */
        final.overall = FINAL_NOT_DETECTED;
        Negative_led_indicate();

        //send to PC
        send_usb_string("{0x08}\n"); //valid ic
        HAL_Delay(20);
        send_usb_string("{0x0A}\n"); //IC  detected
        HAL_Delay(20);

        //send to BLE
        send_BLE_using_USART3_string("{0x08}\n"); //valid ic
        HAL_Delay(20);
        send_BLE_using_USART3_string("{0x0A}\n"); //IC  detected
        HAL_Delay(20);
        Buzzer_Test_Complete_tone();
    }

    return final;
}



static void prv_calculate_baseline(void)
{
    for (uint8_t ch = 0; ch < NUM_CHANNELS; ch++)
    {
        /* Mean */
        float sum = 0.0f;
        for (uint8_t i = 0; i < BASELINE_POINTS; i++)
            sum += s_wells[ch]->Buff[i];
        s_baseline_mean[ch] = sum / (float)BASELINE_POINTS;

        /* Standard deviation */
        float var = 0.0f;
        for (uint8_t i = 0; i < BASELINE_POINTS; i++) {
            float diff = s_wells[ch]->Buff[i] - s_baseline_mean[ch];
            var += diff * diff;
        }
        s_baseline_sd[ch] = sqrtf(var / (float)BASELINE_POINTS);

        /* Threshold: whichever is larger — dynamic SD-based or absolute delta */
        float thr_dynamic  = s_baseline_mean[ch] + THRESHOLD_MULTIPLIER * s_baseline_sd[ch];
        float thr_absolute = s_baseline_mean[ch] + MIN_THRESHOLD_DELTA;
        s_threshold[ch]    = fmaxf(thr_dynamic, thr_absolute);
    }

    s_baseline_locked = true;
}

/**
 * @brief  Detect threshold crossing in a single well's Buff[].
 *         Requires CONSECUTIVE_REQUIRED points above threshold in a row.
 *
 * @param  ch        Channel index
 * @param  up_to     Scan Buff[BASELINE_POINTS .. up_to] only
 *                   (use TOTAL_READINGS-1 for full scan at end of run)
 *
 * @return  tt >= 0  : minute of first crossing (valid Tt)
 *          -1       : no crossing found (NEGATIVE)
 *          -2       : crossing found but outside TT_VALID_MIN..TT_VALID_MAX (INVALID)
 */
static int8_t prv_detect_Tt(uint8_t ch, uint8_t up_to)
{
    float    threshold   = s_threshold[ch];
    uint8_t  consecutive = 0;

    for (uint8_t i = BASELINE_POINTS; i <= up_to; i++)
    {
        if (s_wells[ch]->Buff[i] >= threshold)
        {
            consecutive++;
            if (consecutive >= CONSECUTIVE_REQUIRED)
            {
                uint8_t tt = i - CONSECUTIVE_REQUIRED + 1u;  /* first crossing minute */

                if (tt >= TT_VALID_MIN && tt <= TT_VALID_MAX)
                    return (int8_t)tt;       /* valid positive */
                else
                    return -2;               /* out of window  */
            }
        }
        else
        {
            consecutive = 0;                 /* reset on any dip */
        }
    }

    return -1;   /* no crossing = negative */
}

/**
 * @brief  Apply IC gating using WELL_7 (IC_CHANNEL_INDEX).
 *         Rule:
 *           IC POSITIVE + Target POSITIVE → POSITIVE (report Tt)
 *           IC POSITIVE + Target NEGATIVE → NEGATIVE
 *           IC NEGATIVE + Target POSITIVE → POSITIVE (high load inhibits IC, flag it)
 *           IC NEGATIVE + Target NEGATIVE → INVALID  (extraction/reaction failure)
 */
static void prv_apply_IC_gating(void)
{
    if (s_results[3].result != RESULT_POSITIVE)
    {
        for (uint8_t ch = 0; ch < IC_CHANNEL_INDEX; ch++)
        {
            /* Target negative + IC failed = INVALID (not enough to call negative) */
            if (s_results[ch].result == RESULT_NEGATIVE)
                s_results[ch].result = RESULT_INVALID;

            /* Target positive + IC failed = still POSITIVE
             * (high target load can inhibit IC — this is expected behaviour)
             * Optionally set a flag here to warn the user */
        }
    }
}

/* ─────────────────────────────────────────────
 * PUBLIC API IMPLEMENTATION
 * ───────────────────────────────────────────── */

void PCR_Init(POSITION *well_ptrs[NUM_CHANNELS])
{
    for (uint8_t ch = 0; ch < NUM_CHANNELS; ch++)
        s_wells[ch] = well_ptrs[ch];

    memset(s_results,       0, sizeof(s_results));
    memset(s_baseline_mean, 0, sizeof(s_baseline_mean));
    memset(s_baseline_sd,   0, sizeof(s_baseline_sd));
    memset(s_threshold,     0, sizeof(s_threshold));

    s_baseline_locked = false;
}

void PCR_OnMinuteTick(uint8_t minute)
{
    /* Not enough baseline points yet */
    if (minute < BASELINE_POINTS)
        return;

    /* Lock baseline exactly once */
    if (!s_baseline_locked)
        prv_calculate_baseline();

    /* Early positive check up to current minute */
    for (uint8_t ch = 0; ch < NUM_CHANNELS; ch++)
    {
        if (s_results[ch].done) continue;   /* already called, skip */

        int8_t tt = prv_detect_Tt(ch, minute);

        if (tt > 0)
        {
            s_results[ch].Tt         = tt;
            s_results[ch].result     = RESULT_POSITIVE;
            s_results[ch].end_signal = s_wells[ch]->Buff[minute];
            s_results[ch].fold_rise  = (float)s_wells[ch]->Buff[minute]
                                       / fmaxf(s_baseline_mean[ch], 1.0f);
            s_results[ch].done       = true;
        }
    }

    /* Last minute → run full final analysis */
    if(minute == (SIZE_OF_PHOTO_DATA_CAPTURE - 1u))
    {
        PCR_FinalResults();
        SOMETHING_DETECTED = 1;  //this for when 29 cycles is completed to STOP bLUE LED WHICH IS TOGLLING EVERY SECOND
        PCR_GetFinalRunResult();
    }
}

void PCR_FinalResults(void)
{
    if (!s_baseline_locked)
        return;   /* baseline not ready — nothing to call */

    for (uint8_t ch = 0; ch < NUM_CHANNELS; ch++)
    {
        if (s_results[ch].done) continue;   /* already called positive early */

        int8_t tt = prv_detect_Tt(ch, SIZE_OF_PHOTO_DATA_CAPTURE - 1u);

        s_results[ch].Tt         = tt;
        s_results[ch].end_signal = s_wells[ch]->Buff[SIZE_OF_PHOTO_DATA_CAPTURE - 1u];
        s_results[ch].fold_rise  = (float)s_wells[ch]->Buff[SIZE_OF_PHOTO_DATA_CAPTURE - 1u]
                                   / fmaxf(s_baseline_mean[ch], 1.0f);

        if      (tt > 0)   s_results[ch].result = RESULT_POSITIVE;
        else if (tt == -1) s_results[ch].result = RESULT_NEGATIVE;
        else               s_results[ch].result = RESULT_INVALID;    /* tt == -2 */

        s_results[ch].done = true;
    }

    prv_apply_IC_gating();
}

const WELL_RESULT *PCR_GetResult(uint8_t ch)
{
    if (ch >= NUM_CHANNELS) return NULL;
    return &s_results[ch];
}

float PCR_GetBaselineMean(uint8_t ch)
{
    if (ch >= NUM_CHANNELS) return 0.0f;
    return s_baseline_mean[ch];
}

float PCR_GetThreshold(uint8_t ch)
{
    if (ch >= NUM_CHANNELS) return 0.0f;
    return s_threshold[ch];
}

bool PCR_AllDone(void)
{
    for (uint8_t ch = 0; ch < NUM_CHANNELS; ch++)
        if (!s_results[ch].done) return false;
    return true;
}



void Positive_led_indicate(void)
{
	  HAL_GPIO_WritePin(RB__LED_RED_GPIO_Port, RB__LED_RED_Pin, 0); //off
	  HAL_GPIO_WritePin(RED_LED_GPIO_Port, RED_LED_Pin, 0);  //off
}

void Negative_led_indicate(void)
{
	 HAL_GPIO_WritePin(RB__LED_RED_GPIO_Port, RB__LED_RED_Pin, 0); //off
	 HAL_GPIO_WritePin(Green_LED_GPIO_Port, Green_LED_Pin, 0);  //off
}

void Invalid_led_indicate(void)
{
	 HAL_GPIO_WritePin(RB_LED_BLUE_GPIO_Port, RB_LED_BLUE_Pin, 1); //off
	 HAL_GPIO_WritePin(RB__LED_RED_GPIO_Port, RB__LED_RED_Pin, 0); //off
     HAL_GPIO_WritePin(RED_LED_GPIO_Port, RED_LED_Pin, 0);  //off
     HAL_GPIO_WritePin(Green_LED_GPIO_Port, Green_LED_Pin, 0);  //off
}
