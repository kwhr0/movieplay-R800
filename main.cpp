#include "main.h"
#include "R800.h"
#include <getopt.h>
#include <pthread.h>
#include <SDL2/SDL.h>
#include <algorithm>
#include <string>
#include <vector>

static constexpr uint width = 192, height = 120;
static uint mhz = 0; // 0 for all-out

static R800 cpu;
static uint8_t m[0x10000];
static bool exit_flag;
static FILE *fi;
static Rect area;
static uint area_x, area_y;
static SDL_Surface *surface;

void file_seek(int sector) {
	if (!fi) return;
	fseek(fi, sector << 9, SEEK_SET);
}

int file_getc() {
	if (!fi) return 0;
	return getc(fi);
}

void emu_exit() {
	exit_flag = true;
}

void set_area(Rect *rect) {
	area = *rect;
	area_x = area.left;
	area_y = area.top;
}

void set_data(int data) {
	if (area_x < width && area_y < height) ((uint16_t *)surface->pixels)[(surface->pitch >> 1) * area_y + area_x] = data;
	if (++area_x > area.right) {
		area_x = area.left;
		++area_y;
	}
}

struct Sym {
	Sym(int _adr, const char *_s = "") : adr(_adr), n(0), s(_s) {}
	int adr, n;
	std::string s;
};

static std::vector<Sym> sym;

static bool cmp_adr(const Sym &a, const Sym &b) { return a.adr < b.adr; }
static bool cmp_n(const Sym &a, const Sym &b) { return a.n > b.n; }

static void dumpProfile() {
	if (sym.empty()) return;
	std::sort(sym.begin(), sym.end(), cmp_n);
	int t = 0;
	for (auto i = sym.begin(); i != sym.end() && i->n; i++) t += i->n;
	printf("------\n");
	for (auto i = sym.begin(); i != sym.end() && double(i->n) / t >= 1e-2; i++)
		printf("%4.1f%% %s", 100. * i->n / t, i->s.c_str());
	sym.clear();
}

struct TimeSpec : timespec {
	TimeSpec() {}
	TimeSpec(double t) {
		tv_sec = floor(t);
		tv_nsec = long(1e9 * (t - tv_sec));
	}
	TimeSpec(time_t s, long n) {
		tv_sec = s;
		tv_nsec = n;
	}
	operator double() { return tv_sec + 1e-9 * tv_nsec; }
	TimeSpec operator+(const TimeSpec &t) const {
		time_t s = tv_sec + t.tv_sec;
		long n = tv_nsec + t.tv_nsec;
		if (n < 0) {
			s++;
			n -= 1000000000;
		}
		return TimeSpec(s, n);
	}
	TimeSpec operator-(const TimeSpec &t) const {
		time_t s = tv_sec - t.tv_sec;
		long n = tv_nsec - t.tv_nsec;
		if (n < 0) {
			s--;
			n += 1000000000;
		}
		return TimeSpec(s, n);
	}
};

static void *cpu_thread(void *) {
	int total = 0;
	TimeSpec tstart0, tend0;
	clock_gettime(CLOCK_MONOTONIC, &tstart0);
	while (!exit_flag) {
		if (mhz) {
			constexpr int SLICE = 100;
			TimeSpec tstart, tend;
			clock_gettime(CLOCK_MONOTONIC, &tstart);
			cpu.Execute(1000000L * mhz / SLICE);
			clock_gettime(CLOCK_MONOTONIC, &tend);
			TimeSpec duration = tstart + TimeSpec(1. / SLICE) - tend;
			if (duration.tv_sec >= 0)
				nanosleep(&duration, NULL);
		}
		else {
			cpu.Execute(1000000);
			total++;
		}
		if (!sym.empty()) {
			auto it = upper_bound(sym.begin(), sym.end(), Sym(cpu.GetPC()), cmp_adr);
			if (it != sym.begin() && it != sym.end()) it[-1].n++;
		}
	}
	clock_gettime(CLOCK_MONOTONIC, &tend0);
	if (!mhz) printf("%.0fMHz\n", total / double(tend0 - tstart0));
	return nullptr;
}

static void exitfunc() {
	fclose(fi);
	dumpProfile();
	SDL_Quit();
}

int main(int argc, char *argv[]) {
	if (argc >= 3 && !strncmp(argv[1], "-NS", 3)) {
		argc -= 2;
		argv += 2;
	}
	int c;
	while ((c = getopt(argc, argv, "c:")) != -1)
		switch (c) {
			case 'c':
				sscanf(optarg, "%d", &mhz);
				break;
		}
	if (argc <= optind) {
		fprintf(stderr, "Usage: emu800 [-c <clock freq. [MHz]> (default: %d)] <a.bin file>\n", mhz);
		return 1;
	}
	char path[256];
	strcpy(path, argv[optind]);
	fi = fopen(path, "r");
	if (!fi) {
		fprintf(stderr, "Cannot open %s\n", path);
		return 1;
	}
	uint16_t i = 0;
	while ((c = getc(fi)) != EOF) m[i++] = c;
	fclose(fi);
#if 1
	char *p = strrchr(path, '.');
	if (p) {
		strcpy(p, ".adr");
		fi = fopen(path, "r");
		if (fi) {
			char s[256];
			while (fgets(s, sizeof(s), fi)) {
				int adr;
				if (sscanf(s, "%x", &adr) == 1 && strlen(s) > 5) sym.emplace_back(adr, s + 5);
			}
			fclose(fi);
			sym.emplace_back(0xffff);
		}
	}
#endif
	fi = fopen((std::string(getenv("HOME")) + "/fat.dmg").c_str(), "rb");
	if (!fi) fprintf(stderr, "Warning: disk image cannot open.\n");
	cpu.SetMemoryPtr(m);
	cpu.Reset();
	atexit(exitfunc);
	//
	SDL_Init(SDL_INIT_VIDEO);
	constexpr int mag = 4;
	SDL_Window *window = SDL_CreateWindow("emu800", 100, 100, mag * width, mag * height, 0);
	if (!window) exit(1);
	SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (!renderer) exit(1);
	surface = SDL_CreateRGBSurfaceWithFormat(0, width, height, 16, SDL_PIXELFORMAT_RGB555);
	if (!surface) exit(1);
	pthread_t thread;
	pthread_create(&thread, nullptr, &cpu_thread, nullptr);
	while (!exit_flag) {
		SDL_Event e;
		while (SDL_PollEvent(&e)) {
			switch (e.type) {
				case SDL_QUIT:
					exit_flag = true;
					break;
			}
		}
		SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
		SDL_RenderCopy(renderer, texture, nullptr, nullptr);
		SDL_RenderPresent(renderer);
		SDL_DestroyTexture(texture);
	}
	pthread_join(thread, nullptr);
	return 0;
}
