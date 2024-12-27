#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {
    
    if(strstr(argv[1], "txt") == 0){
        
        int number;
        sscanf(argv[1], "%d", &number);
        number++;

        printf("%d\n", number);
    }
    else{
        FILE *file = fopen(argv[1], "r");
        if (file == NULL) {
            perror("Dosya açýlamadý");
            return 1;
        }

        int number;
        if (fscanf(file, "%d", &number) != 1) {
            fprintf(stderr, "Hata: Dosyadan sayý okunamadý\n");
            fclose(file);
            return 1;
        }
        fclose(file);

        number++;
        printf("%d\n", number);
    }


    

    return 0;
}

