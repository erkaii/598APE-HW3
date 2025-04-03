#include<stdio.h>
#include<stdlib.h>
#include<math.h>

#include <sys/time.h>

#include <thread>
#include <vector>
#include <cstring>

#define MAX_THREAD 4

float tdiff(struct timeval *start, struct timeval *end) {
  return (end->tv_sec-start->tv_sec) + 1e-6*(end->tv_usec-start->tv_usec);
}

struct Planet {
   double mass;
   double x;
   double y;
   double vx;
   double vy;
};

unsigned long long seed = 100;

unsigned long long randomU64() {
  seed ^= (seed << 21);
  seed ^= (seed >> 35);
  seed ^= (seed << 4);
  return seed;
}

double randomDouble()
{
   unsigned long long next = randomU64();
   next >>= (64 - 26);
   unsigned long long next2 = randomU64();
   next2 >>= (64 - 26);
   return ((next << 27) + next2) / (double)(1LL << 53);
}

int nplanets;
int timesteps;
double dt;
double G;

void calculateD(const Planet* __restrict planets, Planet* __restrict nextplanets, int start, int end, int n) {
	for (int i = start; i < end; i++){
		double xi = planets[i].x;
        double yi = planets[i].y;
		double vx = planets[i].vx;
        double vy = planets[i].vy;
		double mass = planets[i].mass;		

        int j = 0;
        for (; j <= n - 4; j += 4) {  
            double dx1 = planets[j].x - xi;
            double dy1 = planets[j].y - yi;
            double distSqr1 = dx1 * dx1 + dy1 * dy1 + 0.0001;
            double invDist1 = mass*planets[j].mass / sqrt(distSqr1);
            double invDist3_1 = invDist1 * invDist1 * invDist1;
            vx += dt * dx1 * invDist3_1;
            vy += dt * dy1 * invDist3_1;

            double dx2 = planets[j + 1].x - xi;
            double dy2 = planets[j + 1].y - yi;
            double distSqr2 = dx2 * dx2 + dy2 * dy2 + 0.0001;
            double invDist2 = mass*planets[j+1].mass / sqrt(distSqr2);
            double invDist3_2 = invDist2 * invDist2 * invDist2;
            vx += dt * dx2 * invDist3_2;
            vy += dt * dy2 * invDist3_2;

            double dx3 = planets[j + 2].x - xi;
            double dy3 = planets[j + 2].y - yi;
            double distSqr3 = dx3 * dx3 + dy3 * dy3 + 0.0001;
            double invDist3 = mass*planets[j+2].mass / sqrt(distSqr3);
            double invDist3_3 = invDist3 * invDist3 * invDist3;
            vx += dt * dx3 * invDist3_3;
            vy += dt * dy3 * invDist3_3;

            double dx4 = planets[j + 3].x - xi;
            double dy4 = planets[j + 3].y - yi;
            double distSqr4 = dx4 * dx4 + dy4 * dy4 + 0.0001;
            double invDist4 = mass*planets[j+3].mass / sqrt(distSqr4);
            double invDist3_4 = invDist4 * invDist4 * invDist4;
            vx += dt * dx4 * invDist3_4;
            vy += dt * dy4 * invDist3_4;
        }

        for (; j < n; j++) {
            double dx = planets[j].x - xi;
            double dy = planets[j].y - yi;
            double distSqr = dx * dx + dy * dy + 0.0001;
            double invDist = mass*planets[j].mass / sqrt(distSqr);
            double invDist3 = invDist * invDist * invDist;
            vx += dt * dx * invDist3;
            vy += dt * dy * invDist3;
        }

		
        nextplanets[i].vx = vx;
        nextplanets[i].vy = vy;
        nextplanets[i].x += dt * vx;
        nextplanets[i].y += dt * vy;
    }
}

Planet* next(Planet* planets) {
	Planet* nextplanets = (Planet*)malloc(sizeof(Planet) * nplanets);
	std::memcpy(nextplanets, planets, sizeof(Planet) * nplanets);
   
	std::vector<std::thread> threads;

	int width = nplanets / MAX_THREAD;
	int extra_width = nplanets % MAX_THREAD;

	int start = 0;
	for (int i = 0; i < MAX_THREAD; i++) {
		int end = start + width + (i < extra_width ? 1:0);

		threads.emplace_back(calculateD, planets, nextplanets, start, end, nplanets);
		
		start = end;
	}

	for (auto &t : threads) t.join();

   free(planets);
   return nextplanets;
}

int main(int argc, const char** argv){
   if (argc < 2) {
      printf("Usage: %s <nplanets> <timesteps>\n", argv[0]);
      return 1;
   }
   nplanets = atoi(argv[1]);
   timesteps = atoi(argv[2]);
   dt = 0.001;
   G = 6.6743;

   Planet* planets = (Planet*)malloc(sizeof(Planet) * nplanets);
   for (int i=0; i<nplanets; i++) {
      planets[i].mass = randomDouble() * 10 + 0.2;
      planets[i].x = ( randomDouble() - 0.5 ) * 100 * pow(1 + nplanets, 0.4);
      planets[i].y = ( randomDouble() - 0.5 ) * 100 * pow(1 + nplanets, 0.4);
      planets[i].vx = randomDouble() * 5 - 2.5;
      planets[i].vy = randomDouble() * 5 - 2.5;

   }

   struct timeval start, end;
   gettimeofday(&start, NULL);
   for (int i=0; i<timesteps; i++) {
      planets = next(planets);
      //printf("x=%f y=%f vx=%f vy=%f\n", planets[nplanets-1].x, planets[nplanets-1].y, planets[nplanets-1].vx, planets[nplanets-1].vy);
   }
   gettimeofday(&end, NULL);
   printf("Total time to run simulation %0.6f seconds, final location %f %f\n", tdiff(&start, &end), planets[nplanets-1].x, planets[nplanets-1].y);

   return 0;   
}
