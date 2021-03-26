/*
 * Implementation of the word_count interface using Pintos lists and pthreads.
 *
 * You may modify this file, and are expected to modify it.
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

#ifndef PINTOS_LIST
#error "PINTOS_LIST must be #define'd when compiling word_count_lp.c"
#endif

#ifndef PTHREADS
#error "PTHREADS must be #define'd when compiling word_count_lp.c"
#endif

#include "word_count.h"

void init_words(word_count_list_t* wclist) {
	pthread_mutex_lock(&(wclist->lock));
	list_init(&(wclist->lst));
	pthread_mutex_unlock(&(wclist->lock));
}

size_t len_words(word_count_list_t* wclist) {
  size_t len = 0;
  struct list_elem *iter;
  pthread_mutex_lock(&(wclist->lock));
  for(iter = list_begin(&(wclist->lst)); iter != list_end(&(wclist->lst)); iter = list_next(iter))
	  len++;
  pthread_mutex_unlock(&(wclist->lock));
  return len;
}

word_count_t* find_word(word_count_list_t* wclist, char* word) {
  struct list_elem *iter;
  word_count_t *tmp;
  
  pthread_mutex_lock(&(wclist->lock));
  for(iter = list_begin(&(wclist->lst)); iter != list_end(&(wclist->lst)); iter = list_next(iter)){
	  tmp = list_entry(iter, word_count_t, elem);
	  if(strcmp(tmp->word, word) == 0){
		pthread_mutex_unlock(&(wclist->lock));
		return tmp;
	  }
  }
   pthread_mutex_unlock(&(wclist->lock));
  return NULL;
}

word_count_t* add_word(word_count_list_t* wclist, char* word) {
struct list_elem *iter;
  word_count_t *tmp;
  int cmp;
  // create new word_count_t
  word_count_t *new_word = (word_count_t*)malloc(sizeof(word_count_t));
  new_word->count = 1;
  new_word->word = (char*)malloc(sizeof(char)*strlen(word)+1);
  strcpy(new_word->word, word);
  
  // NULL list case
  pthread_mutex_lock(&(wclist->lock));
  if(list_empty(&(wclist->lst))){
	list_init(&(wclist->lst));
	// add new element to the beginning of the list
	list_insert(list_begin(&(wclist->lst)), &(new_word->elem));
	pthread_mutex_unlock(&(wclist->lock));
	return new_word;
  }
  else{
	for(iter = list_begin(&(wclist->lst)); iter != list_end(&(wclist->lst)); iter = list_next(iter)){
	  tmp = list_entry(iter, word_count_t, elem);
	  cmp = strcmp(tmp->word, word);
	  if(cmp == 0){
		  (tmp->count)++;
		  // free new word, we did not used
		  free(new_word);
		  pthread_mutex_unlock(&(wclist->lock));
		  return tmp;
	  }
	  else if(cmp > 0){
		// add new element just before the 'iter'
		list_insert(iter, &(new_word->elem));
		pthread_mutex_unlock(&(wclist->lock));
		return new_word;
	  }
	}
	// not returned in for loop. add the word to the end of the list.
	list_insert(list_end(&(wclist->lst)), &(new_word->elem));
	pthread_mutex_unlock(&(wclist->lock));
	return new_word;
  }
  pthread_mutex_unlock(&(wclist->lock));
  return NULL;
}

void fprint_words(word_count_list_t* wclist, FILE* outfile) { 
  struct list_elem *iter;
  word_count_t *tmp;
   pthread_mutex_lock(&(wclist->lock));
  for(iter = list_begin(&(wclist->lst)); iter != list_end(&(wclist->lst)); iter = list_next(iter)){
	  tmp = list_entry(iter, word_count_t, elem);
	  fprintf(outfile, "\t%d\t%s\n", tmp->count, tmp->word);
  }
  pthread_mutex_unlock(&(wclist->lock));
}

static bool less_list(const struct list_elem* ewc1, const struct list_elem* ewc2, void* aux) {
  word_count_t *el1, *el2;
  el1 = list_entry(ewc1, word_count_t, elem);
  el2 = list_entry(ewc2, word_count_t, elem);
  return (el1->count < el2->count) ? true : false;
  //(strcmp(el1->count, el2->count) < 0) ? true : false;
}

void wordcount_sort(word_count_list_t* wclist,
                    bool less(const word_count_t*, const word_count_t*)) {
  pthread_mutex_lock(&(wclist->lock));
  list_sort(&(wclist->lst), less_list, less);
  pthread_mutex_unlock(&(wclist->lock));
}
