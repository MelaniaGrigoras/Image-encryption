#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
typedef struct
{
    unsigned char b, g, r;
} RGB;
#pragma pack(push,1)
typedef struct
{
    char tag[2];
    uint32_t size;
    uint16_t unused1;
    uint16_t unused2;
    uint32_t offset;


    uint32_t nrbytes;
    uint32_t width;
    uint32_t height;
    uint16_t color_planes;
    uint16_t nrbiti;
    uint32_t compression;
    uint32_t raw_size;
    uint32_t pixels_h;
    uint32_t pixels_v;
    uint32_t nrcolor;
    uint32_t important;
} bitmap_header;
#pragma pack(pop)

typedef struct
{
    bitmap_header header;
    RGB *pixels;
} bitmap;
typedef struct
{
    int cifra;
    double corr;
    int sus_x;
    int sus_y;
    int jos_x;
    int jos_y;
} fereastra;
typedef struct
{
    fereastra *vector;
    int size;
} ferestre;
typedef struct
{
   double corr;
}d;
int Xorshift32(unsigned int r)
{
    r=r^r<<13;
    r=r^r>>17;
    r=r^r<<5;
    return r;
}
//Durstenfeld
int random(int k, unsigned int *R)
{

    static int poz=1;
    unsigned int r=R[poz++];
    return r%(k+1);
}

bitmap *load_BMP(char *cale_imagine)
{
    size_t size;
    RGB x;
    FILE *f=fopen(cale_imagine,"rb");
    if(!f)
    {
        perror("Eroare la deschiderea fisierului: ");
        exit(-1);
    }

    bitmap *bmp=malloc(sizeof(bitmap));

    fread(bmp,sizeof(bitmap_header),1,f);

    fseek(f,bmp->header.offset,SEEK_SET);

    bmp->pixels=malloc(bmp->header.raw_size);

    fread(bmp->pixels,bmp->header.raw_size,1,f);

    fclose(f);
    return bmp;
}

void save_BMP(char *cale_imagine, bitmap *bmp)
{
    FILE *fout=fopen(cale_imagine,"wb");
    if(!fout)
    {
        perror("Eroare la deschiderea fisierului: ");
        exit(-1);
    }

    fwrite(&bmp->header, sizeof(bitmap_header),1,fout);
    fwrite(bmp->pixels,bmp->header.raw_size,1,fout);
    fclose(fout);
}

RGB *pixel_adress(int i, bitmap *bmp)
{
    int w=bmp->header.width;
    int nrbytes=w*3;
    int padding=nrbytes%4;

    int i3= 3*i;
    int wi=i3/w;

    return (i3) +wi*padding +  (char*)  bmp->pixels;
}

void schimb(bitmap *bmp, int i, int j)
{
    RGB *pi= pixel_adress(i, bmp);
    RGB *pj= pixel_adress(j, bmp);
    RGB aux;
    aux=*pi;
    *pi=*pj;
    *pj=aux;
}

RGB produs(RGB A, RGB B, RGB C)
{
    RGB rgb;
    rgb.r=A.r^B.r^C.r;
    rgb.g=A.g^B.g^C.g;
    rgb.b=A.b^B.b^C.b;
    return rgb;
}

void criptare(char *cale_imagine_initiala, char *cale_imagine_criptata, char *cheie)
{
    unsigned int i, j, l;
    bitmap *bmp=load_BMP(cale_imagine_initiala);

    FILE *F=fopen(cheie,"r");

    if(!F)
    {
        perror("Eroare la deschiderea fisierului: ");
        exit(-1);
    }

    unsigned int W=bmp->header.width;
    unsigned int H=bmp->header.height;
    unsigned int R0, SV;
    fscanf(F,"%u",&R0);
    fscanf(F,"%u",&SV);

    fclose(F);
    unsigned int *R=malloc(sizeof(unsigned int)*(2*W*H));
    R[0]=R0;

    for(i=1; i<(2*W*H); i++)
        R[i]=Xorshift32(R[i-1]);

    for(i=W*H-1; i>0; i--)
    {
        j=random(i,R);
        schimb(bmp,i,j);
    }

    for(l=0; l<W*H; l++)
    {
        RGB *pk=pixel_adress(l,bmp);

        union
        {
            unsigned int x;
            RGB r;
        } co;

        co.x=R[W*H+l];
        RGB r=co.r;

        if(l==0)
        {
            co.x=SV;
            RGB s=co.r;
            *pk=produs(s,*pk,r);
        }
        else
        {
            RGB *pk1=pixel_adress(l-1,bmp);

            *pk=produs(*pk1,*pk,r);
        }
    }

    save_BMP(cale_imagine_criptata,bmp);
    free(R);
    free(bmp->pixels);
    free(bmp);
}

double *testul_chi_patrat(const char *imagine)
{
    double *chi=malloc(sizeof(double)), F;
    chi[0]=chi[1]=chi[2]=0;
    int i=0, j=0;

    bitmap *bmp=load_BMP(imagine);
    int W= bmp->header.width;
    int H= bmp->header.height;

    F=(double)(W*H)/256;

    int **f=malloc(sizeof(int*)*3);
    f[0]=malloc(sizeof(int)*256);
    f[1]=malloc(sizeof(int)*256);
    f[2]=malloc(sizeof(int)*256);
    for(i=0; i<256; i++)
    {
        f[0][i]=0;
        f[1][i]=0;
        f[2][i]=0;
    }

    for(i=0; i<W*H; i++)
    {
        f[0][bmp->pixels[i].r]++;
        f[1][bmp->pixels[i].g]++;
        f[2][bmp->pixels[i].b]++;
    }
    for(j=0; j<3; j++)
        for(i=0; i<256; i++)
        {
            chi[j]+=((f[j][i]-F)*(f[j][i]-F))/F;
        }
    free(bmp->pixels);
    free(bmp);
    for(i=0; i<3; i++)
        free(f[i]);
    return chi;
}
void grayscale_image(char* nume_fisier_sursa,char* nume_fisier_destinatie)
{
    FILE *fin, *fout;
    unsigned int dim_img, latime_img, inaltime_img;
    unsigned char pRGB[3], header[54], aux;

    printf("nume_fisier_sursa = %s \n",nume_fisier_sursa);

    fin = fopen(nume_fisier_sursa, "rb");
    if(fin == NULL)
    {
        printf("nu am gasit imaginea sursa din care citesc");
        return;
    }

    fout = fopen(nume_fisier_destinatie, "wb+");

    fseek(fin, 2, SEEK_SET);
    fread(&dim_img, sizeof(unsigned int), 1, fin);
    printf("Dimensiunea imaginii in octeti: %u\n", dim_img);

    fseek(fin, 18, SEEK_SET);
    fread(&latime_img, sizeof(unsigned int), 1, fin);
    fread(&inaltime_img, sizeof(unsigned int), 1, fin);
    printf("Dimensiunea imaginii in pixeli (latime x inaltime): %u x %u\n",latime_img, inaltime_img);

    fseek(fin,0,SEEK_SET);
    unsigned char c;
    while(fread(&c,1,1,fin)==1)
    {
        fwrite(&c,1,1,fout);
        fflush(fout);
    }
    fclose(fin);

    //calculam padding-ul pentru o linie
    int padding;
    if(latime_img % 4 != 0)
        padding = 4 - (3 * latime_img) % 4;
    else
        padding = 0;

    printf("padding = %d \n",padding);

    fseek(fout, 54, SEEK_SET);
    int i,j;
    for(i = 0; i < inaltime_img; i++)
    {
        for(j = 0; j < latime_img; j++)
        {
            //citesc culorile pixelului
            fread(pRGB, 3, 1, fout);
            //fac conversia in pixel gri
            aux = 0.299*pRGB[2] + 0.587*pRGB[1] + 0.114*pRGB[0];
            pRGB[0] = pRGB[1] = pRGB[2] = aux;
            fseek(fout, -3, SEEK_CUR);
            fwrite(pRGB, 3, 1, fout);
            fflush(fout);
        }
        fseek(fout,padding,SEEK_CUR);
    }
    fclose(fout);
}
void template_matching(char *I, char *S, float ps)
{
    //schimba l in int
    int ok=0;
    char fi;
    float corr=0;
    int i=0, j=0, x=0, y=0, n;
    FILE *f1=fopen(I,"rb");
    FILE *f2=fopen(S,"rb");
    if(!f1 && !f2)
    {
        perror("Eroare la deschiderea fisierelor: ");
        exit(-1);
    }

    char nume_img_grayscale[] = "sablon.bmp";
    grayscale_image(S, nume_img_grayscale);

    bitmap *bmp1=load_BMP(I);
    int W1= bmp1->header.width;
    int H1= bmp1->header.height;
    bitmap *bmp2=load_BMP(S);
    int W2= bmp2->header.width;
    int H2= bmp2->header.height;
    n=W2*H2;

    for(x=(W1-W2); x>=0; x--)
        for(y=0; y<(H1-H2+1); y++)
        {
            corr=0;
            for(i=0; i<W2; i++)
                for(j=0; j<H2; j++)
                {

                }
            corr=corr/n;
            if(corr>ps)
                ok++;
        }
   // printf(" ok=%d ", ok);
    fclose(f1);
    fclose(f2);
}
void culoare(char *nume_img_grayscale,fereastra fi, RGB C)
{
    int i, j;
    bitmap *bmp=load_BMP(nume_img_grayscale);

    int padding;
    if(bmp->header.width % 4 != 0)
        padding = 4 - (3 *bmp->header.width) % 4;
    else
        padding = 0;

    i=0;
    if(fi.jos_x==0)
        i=fi.sus_y+1;
    else
        i=(bmp->header.width*(fi.jos_x-1))+fi.sus_y+1;
    int poz=bmp->header.width*(fi.jos_x-1)+fi.jos_y+1;
    for(; i<=poz; i++)
    {
        bmp->pixels[i].b=C.b;
        bmp->pixels[i].g=C.g;
        bmp->pixels[i].r=C.r;
    }

    i=0;
    if(fi.sus_x==0)
        i=fi.sus_y+1;
    else
        i=(bmp->header.width*(fi.sus_x-1))+fi.sus_y+1;
    poz=bmp->header.width*(fi.sus_x-1)+fi.jos_y+1;
    for(; i<=poz; i++)
    {
        bmp->pixels[i].b=C.b;
        bmp->pixels[i].g=C.g;
        bmp->pixels[i].r=C.r;
    }

    j=0;
    if(fi.jos_x==0)
        j=fi.sus_y+1;
    else
        j=bmp->header.width*(fi.jos_x-1)+fi.sus_y+1;
    for(i=0; i<(fi.sus_x-fi.jos_x); i++)
    {
        if(i!=0)
            j=j+bmp->header.width;
        bmp->pixels[j].b=C.b;
        bmp->pixels[j].g=C.g;
        bmp->pixels[j].r=C.r;
    }

    j=0;
    if(fi.jos_x==0)
        i=fi.jos_y+1;
    else
        j=bmp->header.width*(fi.jos_x-1)+fi.jos_y+1;
    for(i=0; i<(fi.sus_x-fi.jos_x); i++)
    {
        if(i!=0)
            j=j+bmp->header.width;
        bmp->pixels[j].b=C.b;
        bmp->pixels[j].g=C.g;
        bmp->pixels[j].r=C.r;
    }
    save_BMP(nume_img_grayscale,bmp);
    free(bmp->pixels);
    free(bmp);
}

int cmp(const void *c1, const void *c2)
{
    d *a=(d*)c1;
    d *b=(d*)c2;
    if(a->corr>b->corr)
        return -1;
    return 1;
}
int main()
{
    //Criptografie

   // 11) Scrieți un program care să realizeze următoarele operații: (0.5 puncte)

    //criptează o imagine color BMP și salvează imaginea criptate în memoria
    //externă (căile ambelor imagini și a fișierului text care conține cheia
    //secretă se vor citi de la tastatură sau dintr-un fișier text);
    const char cheie[]="secret_key.txt";
    char cale_imagine[]="peppers.bmp";
    char fisier_extern[]="enc_peppers.bmp";
    criptare(cale_imagine, fisier_extern, cheie);

    //afișează pe ecran valorile testului 𝜒! pentru imaginea inițială și imaginea
    //criptată, pe fiecare canal de culoare.
    double *chi1=testul_chi_patrat(cale_imagine);
    printf("Testul chi patrat pentru imaginea initiala:\nR=%.2lf\nG=%.2lf\nB=%.2lf\n",chi1[0],chi1[1],chi1[2]);
    free(chi1);

    const char imagine_criptata[]="enc_peppers.bmp";
    double *chi2=testul_chi_patrat(imagine_criptata);
    printf("Testul chi patrat pentru imaginea criptata:\nR=%.2lf\nG=%.2lf\nB=%.2lf\n\n",chi2[0],chi2[1],chi2[2]);
    free(chi2);

    //Recunoastere pattern-uri

    char nume_img_sursa[] = "test.bmp";
    char nume_img_grayscale[] = "test_grayscale.bmp";
    grayscale_image(nume_img_sursa, nume_img_grayscale);

    //7
    //furnizează 𝑓! care au corelația mai mare decât pragul 𝑝!
    //  char *I="test.bmp";
    //  const char *S="cifra0.bmp";
    // float ps=0.5;
    //printf(" %d\n",template_matching(I,S,ps));
    //template_matching(I,S,ps);


    //8
    RGB C;
    C.b=0;
    C.g=0;
    C.r=255;

    fereastra fi;
    fi.cifra=5;
    fi.corr=0;
    fi.sus_x=100;
    fi.sus_y=100;
    fi.jos_x=80;
    fi.jos_y=115;

    culoare(nume_img_grayscale,fi,C);

    //9
    int nr_corelatii=5; //nr fictiv
    d detectii[6];
//    detectii=malloc(sizeof(d)*nr_corelatii);
    detectii[0].corr=0.789;
    detectii[1].corr=0.81;
    detectii[2].corr=0.786;
    detectii[3].corr=0.787;
    detectii[4].corr=0.78;

    qsort(detectii, nr_corelatii, sizeof(d), cmp);
    int i;
    printf("\n Sortare detectii:\n");
    for(i=0; i<5; i++)
        printf("%lf ",detectii[i].corr);
    return 0;
}

