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
 * Driver code is in kern/tests/synchprobs.c We will replace that file. This
 * file is yours to modify as you see fit.
 *
 * You should implement your solution to the stoplight problem below. The
 * quadrant and direction mappings for reference: (although the problem is, of
 * course, stable under rotation)
 *
 *   |0 |
 * -     --
 *    01  1
 * 3  32
 * --    --
 *   | 2|
 *
 * As way to think about it, assuming cars drive on the right: a car entering
 * the intersection from direction X will enter intersection quadrant X first.
 * The semantics of the problem are that once a car enters any quadrant it has
 * to be somewhere in the intersection until it call leaveIntersection(),
 * which it should call while in the final quadrant.
 *
 * As an example, let's say a car approaches the intersection and needs to
 * pass through quadrants 0, 3 and 2. Once you call inQuadrant(0), the car is
 * considered in quadrant 0 until you call inQuadrant(3). After you call
 * inQuadrant(2), the car is considered in quadrant 2 until you call
 * leaveIntersection().
 *
 * You will probably want to write some helper functions to assist with the
 * mappings. Modular arithmetic can help, e.g. a car passing straight through
 * the intersection entering from direction X will leave to direction (X + 2)
 * % 4 and pass through quadrants X and (X + 3) % 4.  Boo-yah.
 *
 * Your solutions below should call the inQuadrant() and leaveIntersection()
 * functions in synchprobs.c to record their progress.
 */

#include <types.h>
#include <lib.h>
#include <thread.h>
#include <test.h>
#include <synch.h>

/*
 * Called by the driver during initialization.
 */
static struct semaphore *sem_0;
static struct semaphore *sem_1;
static struct semaphore *sem_2;
static struct semaphore *sem_3;
static struct semaphore *enter;
void
stoplight_init() {
	sem_0 = sem_create("sem_0", 1);
	if (sem_0 == NULL) {
		panic("sem_0 create failed\n");
	}
	sem_1 = sem_create("sem_1", 1);
	if (sem_1 == NULL) {
		panic("sem_1 create failed\n");
	}
	sem_2 = sem_create("sem_2", 1);
	if (sem_2 == NULL) {
		panic("sem_2 create failed\n");
	}
	sem_3 = sem_create("sem_3", 1);
	if (sem_3 == NULL) {
		panic("sem_3 create failed\n");
	}
	enter = sem_create("enter", 2);
	if (enter == NULL) {
		panic("enter create failed\n");
	}
	return;
}

/*
 * Called by the driver during teardown.
 */

void stoplight_cleanup() {
	sem_destroy(sem_0);
	sem_destroy(sem_1);
	sem_destroy(sem_2);
	sem_destroy(sem_3);
	sem_destroy(enter);
	return;
}

void
turnright(uint32_t direction, uint32_t index)
{

	P(enter);
	if(direction==2)
	{
		P(sem_2);
		inQuadrant(direction, index);
		leaveIntersection(index);
		V(sem_2);
	}
	if(direction==3)
	{
		P(sem_3);
		inQuadrant(direction, index);
		leaveIntersection(index);
		V(sem_3);
	}
	if(direction==0)
	{
		P(sem_0);
		inQuadrant(direction, index);
		leaveIntersection(index);
		V(sem_0);
	}
	if(direction==1)
	{
		P(sem_1);
		inQuadrant(direction, index);
		leaveIntersection(index);
		V(sem_1);
	}
	V(enter);
	//(void)direction;
	//(void)index;
	/*
	 * Implement this function.
	 */
	return;
}
void
gostraight(uint32_t direction, uint32_t index)
{
	P(enter);
	if(direction==2)
	{
		P(sem_2);
		P(sem_1);
		inQuadrant(direction, index);
		inQuadrant((direction+3)%4, index);
		leaveIntersection(index);
		V(sem_2);
		V(sem_1);

	}
	if(direction==3)
	{
		P(sem_3);
		P(sem_2);
		inQuadrant(direction, index);
		inQuadrant((direction+3)%4, index);
		leaveIntersection(index);
		V(sem_3);
		V(sem_2);

	}
	if(direction==0)
	{
		P(sem_0);
		P(sem_3);
		inQuadrant(direction, index);
		inQuadrant((direction+3)%4, index);
		leaveIntersection(index);
		V(sem_0);
		V(sem_3);

	}
	if(direction==1)
	{
		P(sem_1);
		P(sem_0);
		inQuadrant(direction, index);
		inQuadrant((direction+3)%4, index);
		leaveIntersection(index);
		V(sem_1);
		V(sem_0);
	}
	V(enter);

	//(void)direction;
	//(void)index;
	/*
	 * Implement this function.
	 */
	return;
}
void
turnleft(uint32_t direction, uint32_t index)
{
	P(enter);
	if(direction==2)
	{
		P(sem_2);
		P(sem_1);
		P(sem_0);
		inQuadrant(direction, index);
		inQuadrant((direction+3)%4, index);
		inQuadrant((direction+2)%4, index);
		leaveIntersection(index);
		V(sem_2);
		V(sem_1);
		V(sem_0);
	}
	if(direction==3)
	{
		P(sem_3);
		P(sem_2);
		P(sem_1);
		inQuadrant(direction, index);
		inQuadrant((direction+3)%4, index);
		inQuadrant((direction+2)%4, index);
		leaveIntersection(index);
		V(sem_3);
		V(sem_2);
		V(sem_1);
	}
	if(direction==0)
	{
		P(sem_0);
		P(sem_3);
		P(sem_2);
		inQuadrant(direction, index);
		inQuadrant((direction+3)%4, index);
		inQuadrant((direction+2)%4, index);
		leaveIntersection(index);
		V(sem_0);
		V(sem_3);
		V(sem_2);
	}
	if(direction==1)
	{
		P(sem_1);
		P(sem_0);
		P(sem_3);
		inQuadrant(direction, index);
		inQuadrant((direction+3)%4, index);
		inQuadrant((direction+2)%4, index);
		leaveIntersection(index);
		V(sem_1);
		V(sem_0);
		V(sem_3);
	}
	V(enter);
	//(void)direction;
	//(void)index;
	/*
	 * Implement this function.
	 */
	return;
}
