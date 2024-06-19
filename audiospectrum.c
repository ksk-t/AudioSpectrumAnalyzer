#include <iio.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <fftw3.h>

static struct iio_context *ctx = NULL;

#define SIGNAL_SIZE 1024
#define REAL 0
#define IMAG 1

int main()
{
   fftw_complex result[SIGNAL_SIZE];
   double input[SIGNAL_SIZE];
  
   fftw_plan plan = fftw_plan_dft_r2c_1d(SIGNAL_SIZE, input, result, FFTW_ESTIMATE);

   ctx = iio_create_default_context();
   struct iio_device *dev = iio_context_find_device(ctx, "TI-am335x-adc.0.auto");
   if (dev == NULL){
      printf("Failed to find ADC device\n"); 
      return -1;
   }else{
      printf("ADC found\n");
   }
   struct iio_channel *ch = iio_device_get_channel(dev, 0);
   if (!ch){
      printf("Could not find channel");
      return -1;
   }
   iio_channel_enable(ch);
   struct iio_buffer *rxbuf = iio_device_create_buffer(dev, SIGNAL_SIZE, false);
	if (!rxbuf) {
      printf("Could not create RX buffer\n");
      return -1;
	}

   ssize_t nbytes_rx = iio_buffer_refill(rxbuf); 
 	ptrdiff_t p_inc = iio_buffer_step(rxbuf);
   char *p_end = iio_buffer_end(rxbuf);
   size_t index = 0;
	for (char *p_dat = (char *)iio_buffer_first(rxbuf, ch); p_dat < p_end; p_dat += p_inc) {
      const uint16_t i = (*((uint16_t*)p_dat));
      input[index] = (double)i; 
      printf("%g\n", input[index]);
      index++;
	}
   
   fftw_execute(plan);

   printf("Frequency Analysis\n");
   double mag[SIGNAL_SIZE/2];
   for(size_t i = 0; i < (SIGNAL_SIZE/2);i++){
      mag[i] = sqrt(result[i][REAL] * result[i][REAL] + result[i][IMAG] * result[i][IMAG]);
   }

   // Normalize
   double max = 0;
   for(size_t i = 0; i < (SIGNAL_SIZE/2);i++){
      if (mag[i] > max){
         max = mag[i];
      }
   }
   int freqBin = 44100 / SIGNAL_SIZE;
   const int HIGHEST = 50;
   for(size_t i = 0; i < (SIGNAL_SIZE/2);i++){
      int normalizedVal = (mag[i] / max) * HIGHEST;
      printf("%d:",freqBin * i);
      for (int j = 0; j < normalizedVal; j++){
         printf(".");
      }
      printf("\n");
   }

   iio_buffer_destroy(rxbuf);
   iio_channel_disable(ch);
   iio_context_destroy(ctx);
   return 0;
}
