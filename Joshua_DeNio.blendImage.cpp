/*
Author: Joshua DeNio
Date: 04/18/2020
Description: This program uses multithreading to smoothe an image.

Contributing authors:  Saeed Salem - 04/13/2020.

To compile: g++ -O3 -w Joshua_DeNio.blendImage.cpp -lpthread -o Joshua_DeNio.blendImage

To run: ./Joshua_DeNio.blendImage InputFile outFile  window_size thread_count
Example: ./Joshua_DeNio.blendImage rover.jpg test.jpg 3 2
*/

#include "stb_image.c"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <pthread.h>
#include "timer.h"

using namespace std;

const int MAX_THREADS = 1024;
int thread_count;

// window size (k)
int window_size;

uint8* src;
uint8* dst;
int width=0;
int height=0;
int actual_comps=0;
int YR = 19595, YG = 38470, YB = 7471;


void smooth_pixel(int y, int x)
{
    int index = 0;
    int red_sum = 0;
    int green_sum = 0;
    int blue_sum = 0;

    // ints for average.
    int avg_red = 0;
    int avg_green = 0;
    int avg_blue = 0;

    int count = 0;
    
    int x2 = 0;
    int y2 = 0;

    // d is half of the window size on each side
    int d = window_size/2;

    //Get a weighted average for the three colors within the window.
    for(int i = -d; i <= d; i++)
    {
      for(int j = -d; j <= d; j++)
      {
        y2 = y + i; 
        x2 = x + j;

        if (y2 < 0 || x2 < 0 || y2 >= height || x2 >= width)
        {
          continue;
        }
        else
        {
          
          index = (y2*width+x2)*actual_comps;
          red_sum = red_sum + src[index];
          green_sum = green_sum + src[index+1];
          blue_sum = blue_sum + src[index+2];
          count++;
        }
        
      }
    }

    // Get the average by dividing sum/count
    avg_red = red_sum/count;
    avg_green = green_sum/count;
    avg_blue = blue_sum/count;

    // Update pixel in the destination image.
    dst[index] = avg_red;
    dst[index+1] = avg_green;
    dst[index+2] = avg_blue;
}

void* Thread_smooth(void* rank){
    //Each thread will do a chunck of the rows (number of rows is height).
  
    // cast the rank to a long
    long my_rank = (long) rank;

    long long my_n = height/thread_count;

    // Start and end possitions for each thread
    int my_start = my_n * my_rank;
    int my_end = (my_start + my_n);

    // If the data can be evenly spread spread it or send the extra to the last thread
    if (my_rank == thread_count - 1)
    {
        my_end = (height % thread_count) + my_end ;
    }

    for (int y = my_start; y < my_end; y++)
    {
        for(int x = 0; x < width; x++)
        {
            // Call smooth_pixel on the target pixel
            smooth_pixel(y, x);
        }
    }
}


int main(int argc, char** argv)
{
  if(argc<2){
    cout<<"To run: ./assign5 fileName outFileName"<<endl;
    cout<<"./assign5 stone.jpeg stoneOut.jpeg"<<endl;
    cout<<"For Multithreading::i \n To run: ./assign5 fileName outFileName thread_count"<<endl;
    cout<<"./assign5 stone.jpeg stoneOut.jpeg 3"<<endl;
    return 0;
  }

  // Declarations for threads.
  long thread;
  pthread_t* thread_handles;

  
     
  const char* pSrc_filename = argv[1];
  const char* pDst_filename = argv[2];

  // Set window size.
  window_size = atoi(argv[3]);

  // Set thread_count.
  thread_count = atoi(argv[4]);

  // allocate memory to the thread handles
  thread_handles = (pthread_t*)malloc(thread_count * sizeof(pthread_t));

  // Doubles for the timer
  double start, finish, elapsed;

  // Load the source image.
  const int req_comps = 3; // request RGB image
  src = stbi_load(pSrc_filename, &width, &height, &actual_comps, req_comps);
  if (!src)
  {
    cout<<"Failed loading file "<<endl;
    return 1;
  }

  cout<<"Source file: "<< pSrc_filename <<" "<< width <<" " <<height <<" "<< actual_comps<<endl;
  cout<<"RGB for pixel 0 (0,0) ::: "<<(int) src[0]<< " "<< (int) src[1]<< " "<< (int) src[2]<<endl;

  //Allocate memory for the output image. 
  dst = (uint8*) malloc(width* height*3);


  //Start timer
  GET_TIME(start);

    // Start threads

    for (thread = 0; thread < thread_count; thread++)
    {
        pthread_create(&thread_handles[thread], NULL, Thread_smooth, (void*)thread);
    }

    // Join the threads
    for (thread = 0; thread < thread_count; thread++)
    {
        pthread_join(thread_handles[thread], NULL);
    }

    // End of threads

  // Finished timeing
  GET_TIME(finish);

  // Set time elapsed
  elapsed = finish - start;

  // Print time elapsed
  cout << "Time elapsed: " << elapsed << " using " << thread_count << " threads" << endl;
  
  if (!stbi_write_tga(pDst_filename, width, height, 3, dst))
  {
    cout<<"Failed writing image to file "<< pDst_filename <<endl;
    free(src);
    free(dst);
    return EXIT_FAILURE;
  }

  free(src);
  free(dst);
  return 0;
}


