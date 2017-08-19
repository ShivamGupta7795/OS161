/*
 * Copyright (c) 2001, 2002, 2009
 *	The President and Fellows of Harvard College.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE UNIVERSITY AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE UNIVERSITY OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*
 * Driver code is in kern/tests/synchprobs.c We will
 * replace that file. This file is yours to modify as you see fit.
 *
 * You should implement your solution to the whalemating problem below.
 */

#include <types.h>
#include <lib.h>
#include <thread.h>
#include <test.h>
#include <synch.h>

/*
 * Called by the driver during initialization.
 */
/*static struct lock *male_lock;
static struct lock *female_lock;
static struct lock *matchmaker_lock;
static struct cv *cv_male;
static struct cv *cv_female;
static struct cv *cv_matchmaker;

volatile unsigned male_count=0;
volatile unsigned female_count=0;
volatile unsigned matchmaker_count=0;*/

static struct semaphore *male_sem;
static struct semaphore *female_sem;

void whalemating_init() {
	male_sem = sem_create("male_sem", 0);
	female_sem = sem_create("female_sem", 0);
	
	/*male_lock = lock_create("male_lock");
	if (male_lock == NULL) {
		panic("male_lock failed\n");
	}
	female_lock = lock_create("female_lock");
	if (female_lock == NULL) {
		panic("female_lock failed\n");
	}
	matchmaker_lock = lock_create("matchmaker_lock");
	if (matchmaker_lock == NULL) {
		panic("matchmaker_lock failed\n");
	}
	cv_male = cv_create("cv_male");
	if (cv_male == NULL) {
		panic("cv_male failed\n");
	}
	cv_female = cv_create("cv_female");
	if (cv_female == NULL) {
		panic("cv_female failed\n");
	}
	cv_matchmaker = cv_create("cv_matchmaker");
	if (cv_matchmaker == NULL) {
		panic("cv_matchmaker failed\n");
	}
	kprintf("Initial: %d %d %d \n", male_count, female_count, matchmaker_count);*/
	return;
}

/*
 * Called by the driver during teardown.
 */

void
whalemating_cleanup() {
	sem_destroy(male_sem);
	sem_destroy(female_sem);
	return;
}

void
male(uint32_t index)
{
	(void)index;
	male_start(index);
	P(male_sem);
	male_end(index);
	/*lock_acquire(male_lock);
	if(female_count==0 || matchmaker_count==0)
	{
		kprintf("male\n");
		male_count++;
		cv_wait(cv_male, male_lock);	
	}
	male_start(index);
	cv_signal(cv_female, male_lock);
	cv_signal(cv_matchmaker, male_lock);
	male_count--;
	male_end(index);
	lock_release(male_lock);*/

	/*
	 * Implement this function by calling male_start and male_end when
	 * appropriate.
	 */
	return;
}

void
female(uint32_t index)
{
	(void)index;
	female_start(index);
	P(female_sem);
	female_end(index);
	/*lock_acquire(female_lock);
	if(male_count==0 || matchmaker_count==0)
	{	
		kprintf("female\n");
		female_count++;
		cv_wait(cv_female, female_lock);	
	}

	female_start(index);
	cv_signal(cv_female, female_lock);
	cv_signal(cv_matchmaker, female_lock);
	female_count--;
	female_end(index);
	lock_release(female_lock);*/
	/*
	 * Implement this function by calling female_start and female_end when
	 * appropriate.
	 */
	return;
}

void
matchmaker(uint32_t index)
{
	(void)index;
	matchmaker_start(index);
	V(male_sem);
	V(female_sem);
	matchmaker_end(index);
	/*lock_acquire(matchmaker_lock);
	if(male_count==0 || female_count==0)
	{
		kprintf("anand");
		matchmaker_count++;
		cv_wait(cv_matchmaker, matchmaker_lock);	
	}
	
	matchmaker_start(index);
	cv_signal(cv_male, matchmaker_lock);
	cv_signal(cv_female, matchmaker_lock);
	matchmaker_count--;
	matchmaker_end(index);
	lock_release(matchmaker_lock);*/
	/*
	 * Implement this function by calling matchmaker_start and matchmaker_end
	 * when appropriate.
	 */
	return;
}
