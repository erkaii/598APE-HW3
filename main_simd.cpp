#include<stdio.h>
#include<stdlib.h>
#include<math.h>

#include <sys/time.h>

#include <thread>
#include <vector>
#include <cstring>
#include <vectorclass.h>

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

		Vec4d xi_v(xi);
		Vec4d yi_v(yi);
		Vec4d mass_v(mass);
		Vec4d dt_v(dt);
		Vec4d eps_v(0.0001);

		Vec4d vx_v(0.0);
		Vec4d vy_v(0.0);

		int j = 0;
		for (; j <= n - 4; j += 4) {  
			Vec4d px(planets[j].x, planets[j + 1].x, planets[j + 2].x, planets[j + 3].x);
			Vec4d py(planets[j].y, planets[j + 1].y, planets[j + 2].y, planets[j + 3].y);
			Vec4d pmass(planets[j].mass, planets[j + 1].mass, planets[j + 2].mass, planets[j + 3].mass);

			Vec4d dx = px - xi_v;
			Vec4d dy = py - yi_v;
			Vec4d distSqr = dx * dx + dy * dy + eps_v;

			Vec4d invDist = (mass_v * pmass) / sqrt(distSqr);
			Vec4d invDist3 = invDist * invDist * invDist;

			vx_v += dt_v * dx * invDist3;
			vy_v += dt_v * dy * invDist3;
		}
		vx += horizontal_add(vx_v);
		vy += horizontal_add(vy_v);

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
		//printf("x=%f y=%f vx=%f vy=%f\n", planets[nplanets-1].x, planets[nplanets-1].y, planets[nplanets-1].vx, planets[nplanets-1].vy);
		planets = next(planets);
	}
	gettimeofday(&end, NULL);
	printf("Total time to run simulation %0.6f seconds, final location %f %f\n", tdiff(&start, &end), planets[nplanets-1].x, planets[nplanets-1].y);

	return 0;   
}
