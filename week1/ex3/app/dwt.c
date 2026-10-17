/* DWT cycle counter — the only honest way to burn a known number of
 * microseconds of CPU inside a task without blocking.
 */
#include "stm32f1xx.h"
#include "dwt.h"
#include "FreeRTOS.h"
void dwt_init(void)
{
    /* TODO 1: enable the trace subsystem (CoreDebug DEMCR, TRCENA bit),
     *         zero DWT->CYCCNT, then enable CYCCNTENA.
     *         PM0056 does not document DWT; see the ARMv7-M ARM, C1.8. */
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk; //Bật quyền hoạt động của khối trace, cho phép dùng DWT.
    DWT->CYCCNT = 0;                                //Reset bộ đếm CYCCNT về 0.
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;            //Bật đếm chu kỳ CPU bằng bit CYCCNTENA.
}

void dwt_busy_wait_us(uint32_t microseconds)
{
    /* TODO 2: spin until `microseconds` worth of CPU cycles have elapsed.
     *         SystemCoreClock is 72000000. Handle CYCCNT wrap-around correctly
     *         — unsigned subtraction, not a comparison of absolute values.
     *         Explain in REPORT.md why the naive comparison breaks every ~59 s. */

    uint32_t cycle_time = (uint32_t)microseconds*(configCPU_CLOCK_HZ/1000000U);
    uint32_t start = DWT->CYCCNT;
    while ((uint32_t)(DWT->CYCCNT - start) < cycle_time){}

}
