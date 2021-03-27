#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <stdio.h>
#include <unistd.h>
#include <fstream>
#include <iostream>
#include <map>
#include <fcntl.h>
#include <assert.h>
using namespace std;

int main(int argc, char *argv[]) {
  if (argc < 2) {
    printf("not enough args\n");
    exit(1);
  }

  ifstream fin(argv[1]);

  int N = atoi(argv[2]);

  int nlines = 0;

  int x, j;

  while (fin >> x) {
    ++nlines;
  }

  // number of lines that each process needs to process
  int nn = (nlines / N) % 2 == 0 ? nlines / N : nlines / N + 1;

  fin.close();

  fin.open(argv[1]);

  // read in the file sequentially in the main process
  // store a temporal file of input for each child process
  for (int i = 0; i < N; ++i) {
    ofstream fout("p" + to_string(i));
    j = 0;
    while (j < nn && fin >> x) {
      fout << x << endl;
      ++j;
    }
    fout.close();
  }
  fin.close();

  int nprocs = 0;
  int cI = 0; //currentIndex

  std::map<int, int> counts;
  std::map<int, int>::iterator it;

  int *mappedMem = (int *) mmap(NULL, N * 4194304,
                                 PROT_READ | PROT_WRITE,
                                 MAP_SHARED | MAP_ANONYMOUS,
                                 -1, 0);
  assert(mappedMem != MAP_FAILED);

//Making pipes
  int pipes[N +1][2];
  int z;
  for (z = 0; z < N + 1; z++) {
    if (pipe(pipes[z]) == -1) {
      printf("Error with created pipe\n");
    }
  }

  while (nprocs < N) {
  // call fork in the loop
  pid_t rc = fork();

  if (rc < 0) {
    exit(1);
  } else if (rc == 0) {  // child (new process)

    for (int pipesj = 0; pipesj <  N +1; pipesj++) {
      //Close unused pipes------------------
      if (nprocs != pipesj) {
        close(pipes[pipesj][0]);
      }
      if (nprocs+1 != pipesj) {
        close(pipes[pipesj][1]);
      }
    }
      //number given in this pipe is index to start on.
      if (read(pipes[nprocs][0], &cI, sizeof(int)) == -1) {
        printf("Error at reading\n");
        exit(1);
      }

    // each child process reads in its input and count the numbers
    ifstream fin("p" + to_string(nprocs));
    while (fin >> x) {
      if (counts.find(x) == counts.end()) counts[x] = 0;
      ++counts[x];
    }
    fin.close();

    remove(("p" + to_string(nprocs)).c_str());
    // store the results to shared memory
    for (auto &c : counts) {
      mappedMem[cI] = c.first;
      cI++;
      mappedMem[cI] = c.second;
      cI++;
    }

    //sends line number to start on for next process
    if (write(pipes[nprocs + 1][1], &cI, sizeof(int)) == -1) {
      exit(1);
    }
    close(pipes[nprocs][0]);
    close(pipes[nprocs+1][1]);

    return 0;

    } else {  // parent goes down this path (main)
      // wait function should not be invoked here
      if (write(pipes[0][1], &cI, sizeof(int)) == -1) {
        printf("Error at writing\n");
        exit(1);
      }
      ++nprocs;
    }
  }

 // wait function can only be invoked after all processes are created
  // call wait or waitpid for nproces times to make sure all child processes have returned
  while (nprocs > 0) {
    wait(NULL);
    --nprocs;
  }

  if (read(pipes[N][0], &cI, sizeof(int)) == -1) {
    printf("error at reading partent pipe\n");
    exit(1);
  }
  // the main process merges the results for child processes
  int i = 0;
  int total;
  while (i < cI) {
    it = counts.find(mappedMem[i]);
    if (it != counts.end()) {
      total = (it->second + mappedMem[i+1]);
      counts.erase (it);
      counts.insert(pair<int, int>(mappedMem[i], total));
      i = i + 2;
    } else {
      counts.insert(pair<int, int>(mappedMem[i], mappedMem[i+1]));
      i = i + 2;
   }
  }

  for (int i = 0; i < N; ++i) {
     remove(("cp" + to_string(i)).c_str());
  }

  for (auto &c : counts) {
    cout << c.first << " " << c.second << endl;
  }

  munmap(&mappedMem, N * 4194304);

  return 0;
}