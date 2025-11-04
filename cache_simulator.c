#include<stdio.h>
#include<stdlib.h>

#define MAX_FRAMES 4     // Capacity (c)
#define MAX_PAGES 12     // Number of page requests

//--------------------Clock Algo Implementation----------------------
void simulateClock(int pages[])
{
     int buffer[MAX_FRAMES];
     int refBit[MAX_FRAMES];
     int pointer = 0, hits = 0, misses = 0;

     for (int i = 0; i < MAX_FRAMES; i++)
     {
          buffer[i] = -1;
          refBit[i] = 0;
     }

     printf("Clock: \n");

     for (int i = 0; i < MAX_PAGES; i++)
     {
          int page = pages[i];
          int found = 0;

          for (int j = 0; j < MAX_FRAMES; j++)
          {
               if (buffer[j] == page)
               {
                    found = 1;
                    refBit[j] = 1;
                    hits++;
                    break;
               }
          }

          if (!found)
          {
               misses++;
               while (1)
               {
                    if (refBit[pointer] == 0)
                    {
                         buffer[pointer] = page;
                         refBit[pointer] = 0;
                         pointer = (pointer + 1) % MAX_FRAMES;
                         break;
                    }
                    else
                    {
                         refBit[pointer] = 0;
                         pointer = (pointer + 1) % MAX_FRAMES;
                    }
               }
          }

          // printf("Request %2d -> ", page);
          // for (int j = 0; j < MAX_FRAMES; j++)
          //      if (buffer[j] != -1)
          //           printf("%d", buffer[j]);

          // printf("\n");
     }
     printf("\n---------Clock Results------------\n");
     printf("\nTotal Hits: %d\nTotal Misses: %d\nHit Ratio: %.2f\n",
            hits, misses, (float)hits / (hits + misses));
}

// -----------------LRU Implementation-----------------------------------
int findPage(int page, int buffer[], int n)
{
     for(int i=0; i<n; i++)
     {
          if(buffer[i] == page)
          {
               return i;
          }
     }
     return -1;
}

void simulateLRU(int pages[]){
     int buffer[MAX_FRAMES];
     int time[MAX_FRAMES];
     int Hits = 0, Miss = 0, counter = 0;

     for(int i=0; i<MAX_FRAMES; i++)
     {
          buffer[i] = -1;
          time[i] = 0;
     }

     for(int i=0; i<MAX_PAGES; i++)
     {
          int page = pages[i];
          int pos = findPage(page, buffer, MAX_FRAMES);
          if(pos != -1)
          {
               Hits++;
               time[pos] = ++counter;
          }
          else
          {
               Miss++;
               int empty = -1;
               for(int j=0; j<MAX_FRAMES; j++)
               {
                    if(buffer[j] == -1)
                    {
                         empty = j;
                         break;
                    }
               }

               if(empty != -1)
               {
                    buffer[empty] = page;
                    time[empty] = ++counter;
               }
               else
               {
                    int lru = 0;
                    for(int j=1; j<MAX_FRAMES; j++)
                    {
                         if(time[j] < time[lru])
                         {
                              lru = j;
                         }
                    }
                    buffer[lru] = page;
                    time[lru] = ++counter;
               }
          }
          // printf("Requested page: %d -> ", page);
          // for(int j=0; j<MAX_FRAMES; j++)
          // {
          //      if(buffer[j]!=-1)
          //       printf("%d ", buffer[j]);
          // }
          // printf("\n");
     }
     printf("\n---------LRU Results------------\n");
     printf("Hits: %d\nMisses: %d\nHit Ratio: %.2f\n", Hits, Miss, (float)Hits/(Hits+Miss));
}

//------------------------ARC(Adaptive Repalacement Cache)--------------------------------
typedef struct {
    int arr[2 * MAX_FRAMES]; // allow extra for ghost lists
    int size;
} List;

// Helper functions
int find(List *L, int page) {
    for (int i = 0; i < L->size; i++)
        if (L->arr[i] == page)
            return i;
    return -1;
}

void remove_at(List *L, int index) {
    for (int i = index; i < L->size - 1; i++)
        L->arr[i] = L->arr[i + 1];
    L->size--;
}

void insert_front(List *L, int page) {
    for (int i = L->size; i > 0; i--)
        L->arr[i] = L->arr[i - 1];
    L->arr[0] = page;
    L->size++;
}

void move_to_front(List *L, int index) {
    int page = L->arr[index];
    remove_at(L, index);
    insert_front(L, page);
}

// ARC Simulation
void simulate_ARC(int pages[], int n) {
    List T1 = { .size = 0 }, T2 = { .size = 0 }, B1 = { .size = 0 }, B2 = { .size = 0 };
    int p = 0; // adaptive target for T1 size
    int hits = 0, misses = 0;

    for (int i = 0; i < n; i++) {
        int page = pages[i];
        //printf("\nRequest %d:\n", page);

        // CASE 1: Hit in T1 or T2
        int i1 = find(&T1, page);
        int i2 = find(&T2, page);
        if (i1 != -1 || i2 != -1) {
            hits++;
            //printf("  -> HIT in %s\n", i1 != -1 ? "T1" : "T2");

            // Move to T2 front (frequent)
            if (i1 != -1) {
                remove_at(&T1, i1);
                insert_front(&T2, page);
            } else {
                move_to_front(&T2, i2);
            }
        }

        // CASE 2: Hit in B1 (ghost list)
        else if ((i1 = find(&B1, page)) != -1) {
            misses++;
            //printf("  -> Ghost HIT in B1 (recency important)\n");
            // Increase p
            int inc = (B2.size == 0) ? 1 : (B2.size / B1.size);
            if (inc < 1) inc = 1;
            p = (p + inc > MAX_FRAMES) ? MAX_FRAMES : p + inc;

            // Replace before adding
            if (T1.size > 0 && T1.size > p) {
                int victim = T1.arr[T1.size - 1];
                remove_at(&T1, T1.size - 1);
                insert_front(&B1, victim);
            } else if (T2.size > 0) {
                int victim = T2.arr[T2.size - 1];
                remove_at(&T2, T2.size - 1);
                insert_front(&B2, victim);
            }

            // Move page from B1 to T2
            remove_at(&B1, i1);
            insert_front(&T2, page);
        }

        // CASE 3: Hit in B2 (ghost list)
        else if ((i1 = find(&B2, page)) != -1) {
            misses++;
           // printf("  -> Ghost HIT in B2 (frequency important)\n");
            // Decrease p
            int dec = (B1.size == 0) ? 1 : (B1.size / B2.size);
            if (dec < 1) dec = 1;
            p = (p - dec < 0) ? 0 : p - dec;

            // Replace before adding
            if (T1.size > 0 && T1.size > p) {
                int victim = T1.arr[T1.size - 1];
                remove_at(&T1, T1.size - 1);
                insert_front(&B1, victim);
            } else if (T2.size > 0) {
                int victim = T2.arr[T2.size - 1];
                remove_at(&T2, T2.size - 1);
                insert_front(&B2, victim);
            }

            // Move page from B2 to T2
            remove_at(&B2, i1);
            insert_front(&T2, page);
        }

        // CASE 4: Miss (new page)
        else {
            misses++;
            //printf("  -> MISS (new page)\n");

            // If total exceeds cache size
            if (T1.size + T2.size >= MAX_FRAMES) {
                if (T1.size > p) {
                    int victim = T1.arr[T1.size - 1];
                    remove_at(&T1, T1.size - 1);
                    insert_front(&B1, victim);
                } else {
                    int victim = T2.arr[T2.size - 1];
                    remove_at(&T2, T2.size - 1);
                    insert_front(&B2, victim);
                }
            }

            // Insert new page into T1
            insert_front(&T1, page);
        }

        // Limit ghost sizes to cache size
        if (B1.size > MAX_FRAMES) remove_at(&B1, B1.size - 1);
        if (B2.size > MAX_FRAMES) remove_at(&B2, B2.size - 1);
    }

    printf("\n-------ARC Results--------------------\n");
    printf("Total Hits   : %d\n", hits);
    printf("Total Misses : %d\n", misses);
    printf("Hit Ratio    : %.2f%%\n", (hits * 100.0) / n);
}


int main(){
     int pages[MAX_PAGES] = {1, 2, 3, 4, 1, 2, 5, 1, 2, 3, 4, 5};
     simulateLRU(pages);
     simulateClock(pages);
     simulate_ARC(pages, MAX_PAGES);
     return 0;
}