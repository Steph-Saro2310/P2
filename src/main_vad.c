#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sndfile.h>

#include "vad.h"
#include "vad_docopt.h"

#define DEBUG_VAD 0x1        /* Habilita la depuración para el VAD si es necesario */

/* Función principal para procesar la señal de voz y realizar VAD */
int main(int argc, char *argv[]) {
  int verbose = 0; /* Para mostrar el estado interno del VAD: verbose = DEBUG_VAD; */

  SNDFILE *sndfile_in, *sndfile_out = 0;
  SF_INFO sf_info;
  FILE *vadfile;
  int n_read = 0, i;

  VAD_DATA *vad_data;
  VAD_STATE state, last_state;

  float *buffer, *buffer_zeros;
  int frame_size;         /* en muestras */
  float frame_duration;   /* en segundos */
  unsigned int t, last_t; /* en frames */

  char	*input_wav, *output_vad, *output_wav;

  DocoptArgs args = docopt(argc, argv, /* ayuda */ 1, /* versión */ "2.0");

  verbose    = args.verbose ? DEBUG_VAD : 0;
  input_wav  = args.input_wav;
  output_vad = args.output_vad;
  output_wav = args.output_wav;
  float alfa1 = atof(args.alfa1);

  if (input_wav == 0 || output_vad == 0) {
    fprintf(stderr, "%s\n", args.usage_pattern);
    return -1;
  }

  /* Abrir archivo de entrada */
  if ((sndfile_in = sf_open(input_wav, SFM_READ, &sf_info)) == 0) {
    fprintf(stderr, "Error al abrir el archivo de entrada %s (%s)\n", input_wav, strerror(errno));
    return -1;
  }

  if (sf_info.channels != 1) {
    fprintf(stderr, "Error: el archivo de entrada debe ser mono: %s\n", input_wav);
    return -2;
  }

  /* Abrir archivo para guardar los resultados de VAD */
  if ((vadfile = fopen(output_vad, "wt")) == 0) {
    fprintf(stderr, "Error al abrir el archivo de salida VAD %s (%s)\n", output_vad, strerror(errno));
    return -1;
  }

  /* Abrir archivo WAV de salida con el mismo formato, canales, etc. que el archivo de entrada */
  if (output_wav) { //mod
    if ((sndfile_out = sf_open(output_wav, SFM_WRITE, &sf_info)) == 0) { //mod
      fprintf(stderr, "Error al abrir el archivo de salida WAV %s (%s)\n", output_wav, strerror(errno));
      return -1;
    }
  }

  /* Inicializar VAD */
  vad_data = vad_open(sf_info.samplerate);
  /* Asignar memoria para los buffers */
  frame_size   = vad_frame_size(vad_data);
  buffer       = (float *) malloc(frame_size * sizeof(float));
  buffer_zeros = (float *) malloc(frame_size * sizeof(float));
  for (i = 0; i < frame_size; ++i) buffer_zeros[i] = 0.0F;

  frame_duration = (float) frame_size / (float) sf_info.samplerate;
  last_state = ST_UNDEF;

  /* Procesar cada frame de la señal */
  for (t = last_t = 0; ; t++) {
    /* Terminar el bucle cuando el archivo se haya leído completamente */
    if ((n_read = sf_read_float(sndfile_in, buffer, frame_size)) != frame_size) break;

    if (sndfile_out != 0) { //mod
      /* Copiar todas las muestras en el archivo de salida */
      sf_write_float(sndfile_out, buffer, n_read); //mod
    }

    /* Realizar la detección de actividad de voz */
    state = vad(vad_data, buffer, alfa1);
    if (verbose & DEBUG_VAD) vad_show_state(vad_data, stdout);

    /* Imprimir solo las etiquetas SILENCE y VOICE */
    if (state != last_state) {
      if (t != last_t)
        fprintf(vadfile, "%.5f\t%.5f\t%s\n", last_t * frame_duration, t * frame_duration, state2str(last_state));
      last_state = state;
      last_t = t;
    }

    if (sndfile_out != 0) { //mod
      /* Volver atrás y escribir ceros en los segmentos de silencio */
      if (state == ST_SILENCE) { //mod
        sf_seek(sndfile_out, -n_read, SEEK_CUR); //mod
        sf_write_float(sndfile_out, buffer_zeros, n_read); //mod
      }
    }
  }

  /* Imprimir la última parte de la señal */
  state = vad_close(vad_data);
  if (t != last_t)
    fprintf(vadfile, "%.5f\t%.5f\t%s\n", last_t * frame_duration, t * frame_duration + n_read / (float) sf_info.samplerate, state2str(state));

  /* Liberar memoria y cerrar archivos */
  free(buffer);
  free(buffer_zeros);
  sf_close(sndfile_in);
  fclose(vadfile);
  if (sndfile_out) sf_close(sndfile_out);
  
  return 0;
}
