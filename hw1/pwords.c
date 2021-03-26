/*
 * Word count application with one thread per input file.
 *
 * You may modify this file in any way you like, and are expected to modify it.
 * Your solution must read each input file from a separate thread. We encourage
 * you to make as few changes as necessary.
 */

/*
 * Copyright © 2021 University of California, Berkeley
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <pthread.h>

#include "word_count.h"
#include "word_helpers.h"

typedef struct list_and_file_name {
	word_count_list_t* wc;
	char* file_name;
} list_and_file;

void* threadfun(void* thread_var){
	list_and_file* lf = (list_and_file*) thread_var;
	FILE* fp = fopen(lf->file_name,"r");
	count_words(lf->wc, fp);
	fclose(fp);
	pthread_exit(0);
}

int main(int argc, char* argv[]) {
  /* Create the empty data structure. */
  word_count_list_t word_counts;
  init_words(&word_counts);
  if (argc <= 1) {
    /* Process stdin in a single thread. */
    count_words(&word_counts, stdin);
  } else {
		int num_args = argc - 1;
		int i;
		pthread_t tids[num_args];
		//word_counts->mutex = PTHREAD_MUTEX_INITIALIZER;
		pthread_mutex_init(&(word_counts.lock),NULL);
		list_and_file lfs[num_args];
		
		for(i = 0; i < num_args; i++){
			(lfs[i]).wc = &word_counts;
			(lfs[i]).file_name = (char*) malloc(strlen(argv[i+1])+1);
			strcpy((lfs[i]).file_name, argv[i+1]);
			pthread_create(&tids[i], NULL, threadfun, &lfs[i]);
		}
		for(i = 0; i < num_args; i++)
			pthread_join(tids[i], NULL);
  }

  /* Output final result of all threads' work. */
  wordcount_sort(&word_counts, less_count);
  fprint_words(&word_counts, stdout);
  return 0;
}
