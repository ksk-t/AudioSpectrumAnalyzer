#include <iio.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <fftw3.h>

static struct iio_context *ctx = NULL;

#define SIGNAL_SIZE 1024
#define REAL 0
#define IMAG 1

void draw_bar(const int height, const int max_height, int column){
   for (int i = 1; i <= height; i++){
      //printf("\x1b[%d;%dH", max_height - height + i, column);
      printf("|");
   }
   printf("\n");
}

void draw_histogram(int* yvalue, int* values, size_t size, int max_height)
{ 
   printf("\x1b[2J");
   for (size_t i = 0; i < size; i++){
      printf("%d:\t", yvalue[i]);
      draw_bar(values[i], max_height, i + 1);
   }
}

void binize(int* bin_values, size_t bin_size, int* freq, double* power, size_t freq_spectrum_size, double *output)
{
   size_t curr_offset = 0;
   for (size_t i = 0; i < bin_size; i++)
   {
      double curr_total = 0;
      size_t index = 0;
      double max = 0;
      while(((index + curr_offset) < freq_spectrum_size) && (freq[curr_offset + index] <= bin_values[i]))
      {
//          printf("freq:%d index:%d:%g\n", freq[curr_offset + index], curr_offset + index ,power[curr_offset + index]);
         if (power[curr_offset + index] > max)
         {
            max = power[curr_offset + index];
         }
         index++;
      }
//       printf("BinValue: %d, Index: %u, Total: %g, offset: %d\n", bin_values[i], index, curr_total, curr_offset);
      output[i] = max;
      curr_offset += index;
   }
}

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
      index++;
	}
   
   fftw_execute(plan);

   double mag[SIGNAL_SIZE/2];
   mag[0] = 0; // There will almost always be a DC offset which causes the power at 0Hz to be quite high. Ignore this offset
   int freqBin = 44100 / SIGNAL_SIZE;
   for(size_t i = 1; i < (SIGNAL_SIZE/2);i++){
      mag[i] = sqrt(result[i][REAL] * result[i][REAL] + result[i][IMAG] * result[i][IMAG]);
//       printf("%d: %g\n", freqBin*i , mag[i]);
   }

   // Normalize
   int freqs[SIGNAL_SIZE/2];
   for(size_t i = 0; i < (SIGNAL_SIZE/2);i++){
      freqs[i] = freqBin * i;
   }
  
   double max = 0;
   for (size_t i = 0; i < (SIGNAL_SIZE/2); i++)
   {
      if (mag[i] > max)
         max = mag[i];
   }

   for (size_t i = 0; i < (SIGNAL_SIZE/2); i++)
   {
      mag[i] = (mag[i] / max) * 50;
//       printf("%d: %g\n", freqBin*i , mag[i]);
      
   }

   int bins[] = {10, 30, 50, 75, 100, 125, 1000, 2000, 4000, 10000, 16000, 20000};
   double output[12];
   binize(bins, sizeof(bins) / sizeof(bins[0]),  freqs, mag, SIGNAL_SIZE/2, output);
   int outputInt[12];
   for (size_t i = 0; i < 12; i++){
      outputInt[i] = (int)(output[i]);
      outputInt[i] += 1;
//       printf("%d\n", outputInt[i]);
   }

   draw_histogram(bins, outputInt, 12, 50);

   // Clean up
   iio_buffer_destroy(rxbuf);
   iio_channel_disable(ch);
   iio_context_destroy(ctx);
   return 0;
}
