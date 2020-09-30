#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
void dosya_deflate(FILE *f);
char *dosya_oku(FILE *f,int *uzunluk);
int eslesme_uzunlugu(char *string1, char *string2, int limit);
void frekanslar(char *str, FILE *f);
int kaydirma_uzunlugu(int a, int b);
struct belirtec *lz77kodlanmis(char *metin, int limit, int *belirtec_say, FILE *f);
void lzsskodlanmis(char *metin, int limit, int *belirtec_say, FILE *f);
void huffmani_calistir(char harfler[], int frekanslari[], int n, char orijinal_cumle[], FILE *f);

struct belirtec
{
    uint8_t kay_miktar; //isaretsiz 8 bit uzunluğundaki bir integer
    char c;
} typedef belirt;

struct dugum
{
    char harf;
    int frekansi;
    struct dugum *sol;
    struct dugum *sag;
} typedef dugum;

dugum * yigin[100]; //yigin tanimlama
int yigin_size=0; //yigin buyuklugu


//int n[100];
//int g = 0 ;

//char harfler[100][100];
int y = 0 ;
int kaydirma_bit;
int max_kaydirma_bit;
int uzunluk_bit;
int max_uzunluk_bit;
int deflate_toplam=0;
int deflate_toplam_byte;
char lzss_cumlesi[2000]; //LZ77 cümlesi oluşturma (LZ77)
char lz77_cumlesi[2000]; //LZSS cümlesi oluşturma (DEFLATE)
int q_lzss=0; //lzss cümlesini oluşturmak için kullanılan sayaç
int q_lz77=0; //lzss cümlesini oluşturmak için kullanılan sayaç
//aşağıdaki LZ77 ve LZSS kod parçaları https://ysar.net/algoritma/lz77.html sitesinden uyarlanmıştır.
//aşağıdaki Huffman Kodlama parçaları https://www.thedailyprogrammer.com/2015/03/huffman-encoding.html sitesinden uyarlanmıştır.
int main()
{
    printf("\t----LZ77 ve DEFLATE metodlarin incelenmesi----\t\n\n");

    kaydirma_bit=5;
    max_kaydirma_bit = ((1 << kaydirma_bit) -1);
    uzunluk_bit = (8 - kaydirma_bit);
    max_uzunluk_bit = (1 << uzunluk_bit) -1;
    int boyut_lz77=0, boyut_lzss=0;
    FILE *f;
    char *kaynak_metin;
    int metin_boyutu = 8 ;
    if(f = fopen("kaynak.txt","rb")) //kaynak.txt metni okuma modunda açılıyor.
    {
        kaynak_metin = dosya_oku(f,&metin_boyutu);
        fclose(f);
    }
    kaynak_metin[karakter_sayisi(f)]='\0';
    int bel_sayisi_lz77; //lz77 ve lzss şifrelenme modların belirteç sayacı oluşturma.
    int bel_sayisi_lzss;
    belirt *kodlanmis_lz77 = lz77kodlanmis(kaynak_metin,metin_boyutu,&bel_sayisi_lz77, f); // LZ77 kodlanmaya gönderiliyor.
    //belirt *kodlanmis_lzss = lzsskodlanmis(kaynak_metin, metin_boyutu, &bel_sayisi_lzss);

    f=fopen("LZ77sonuc.txt", "w"); //sonuc dosyası açılıyor.
    printf("\nLZ77 metoduyla kodlandiktan sonra: ");
    fputs("-----------LZ77 METODUNUN SIFRELENMESI-----------\n\nVERI [POINTERSIZ]: ", f);
    for(int i = 0 ; i < bel_sayisi_lz77; i++)
    {
        //printf("%c",kodlanmis_lz77[i].c);
        boyut_lz77+=sizeof(kodlanmis_lz77[i].c);
        fputc(kodlanmis_lz77[i].c, f);
    }
    fputs("\n", f);
    // printf("\n");
    printf("%s\n", lz77_cumlesi);
    fprintf(f, "VERI [POINTERLI]: %s\n", lz77_cumlesi);
    printf("--- NORMAL BOYUT %d, SIFRLENDIKTEN SONRAKI: %d ---\n\n", metin_boyutu, boyut_lz77);
    fprintf(f, "\nSIFRELENMEDEN ONCE BOYUT: %d byte\nSIFRLENDIKTEN SONRA BOYUT: %d byte\n\n", metin_boyutu, boyut_lz77);
    fseek(f, 0, SEEK_END); //sonuç dosyasının sona metin eklemek için yazdırılan kısmının sonuna gidiliyor.


    dosya_deflate(f);  //deflate şifrelenmesine gönderiliyor.
    deflate_toplam_byte=deflate_toplam/8;
    //karşılaştırma yapılır.
    if(boyut_lz77 > deflate_toplam_byte)
    {
        printf("\n\nMETININ LZ77 ILE SIFRELENMIS BOYUTU: %d\n",boyut_lz77);
        printf("METININ DEFLATE ILE SIFRELENMIS BOYUTU: %d\n",deflate_toplam_byte);
        fprintf(f,"METININ DEFLATE ILE SIFRELENMIS BOYUTU: %d\n",deflate_toplam_byte);
        printf("\nSONUC :  DEFLATE ALGORITMASI LZ77 ALGORITMASINDAN DAHA IYI SIKISTIRMA YAPAR.\n");
    }
    else
    {
        printf("\n\nMETININ LZ77 ILE SIFRELENMIS BOYUTU: %d\n",boyut_lz77);
        printf("METININ DEFLATE ILE SIFRELENMIS BOYUTU: %d\n",deflate_toplam_byte);
        printf("\nSONUC :  LZ77 ALGORITMASI DEFLATE ALGORITMASINDAN DAHA IYI SIKISTIRMA YAPAR.\n");
    }
    fclose(f);

    devam();
    return 0;
}
void devam()
{
    printf("DEVAM ETMEK ICIN HERHANGI BIR TUSA BASINIZ...\n");
    getch();
}
int karakter_sayisi(FILE *f)
{
    //dosyanın her karakteri okunur ve karakter sayısı bulunur.
    f=fopen("kaynak.txt", "r");
    int count=0;
    char c;
    for (c = getc(f); c != EOF; c = getc(f))
        count = count + 1;
    fclose(f);
    return count;
}
char *dosya_oku(FILE *f,int *uzunluk)
{
    //dosya okunuyor
    char *icerik;
    fseek(f,0,SEEK_END);
    *uzunluk = ftell(f);
    icerik = malloc(*uzunluk);
    fseek(f,0,SEEK_SET);
    fread(icerik,1,*uzunluk,f);
    return icerik;
}
int kaydirma_uzunlugu(int a, int b)
{
    //kaydırma uzunluğu belirtiliyor.
    return (a << uzunluk_bit | b );
}
int eslesme_uzunlugu(char *string1, char *string2, int limit)
{
    //lz77 modunda arama tamponunda bulunan benzetmenin uzunluğunu bulunuyor.
    int uzunluk;
    for(uzunluk = 0 ; *string1++ == *string2++ && uzunluk < limit ; uzunluk++);
    return uzunluk;
}
struct belirtec *lz77kodlanmis(char *metin, int limit, int *belirtec_say, FILE *f)
{
    int bel_say = 0 ;
    int kapasite = 1 << 3 ; //1 sayisini 3 bit sola kaydır
    belirt b; // Belirteç oluşturuyoruz.
    char *ileri_tampon;
    char *arama_tampon;
    int i_kaydirma=1;
    f=fopen("LZ77sonuc.txt", "a");

    printf("\t\t ----- LZ77 sifrelenmesi ---- \n\n");
    belirt *kodla = malloc(kapasite*sizeof(belirt));
    for(ileri_tampon = metin ; ileri_tampon < metin+limit ; ileri_tampon++)
    {

        arama_tampon = ileri_tampon - max_kaydirma_bit;

        if(arama_tampon< metin)
        {
            arama_tampon = metin;
        }

        int eslesme_boyutu = 0 ;
        char *eslesme = ileri_tampon;

        for(; arama_tampon < ileri_tampon ; arama_tampon++)
        {

            int uzunluk = eslesme_uzunlugu(arama_tampon,ileri_tampon,max_uzunluk_bit);

            if(uzunluk > eslesme_boyutu)
            {

                eslesme_boyutu = uzunluk;
                eslesme = arama_tampon;

            }
        }

        if(ileri_tampon + eslesme_boyutu >= metin + limit)
        {

            eslesme_boyutu = metin + limit-ileri_tampon-1; //eğer eşleşmenin son karekteri de dahil ise eşletmeyi kısalt.
        }
        char benzetme[10];
        if((ileri_tampon-eslesme)!=0)
        {
            //eğer eşlesme bulunduysa pointer oluşturuluyor.
            sprintf(benzetme, " <%d,%d,%c> ", ileri_tampon-eslesme, eslesme_boyutu, ileri_tampon[eslesme_boyutu]);
            strcat(lz77_cumlesi,benzetme);
            q_lz77=strlen(lz77_cumlesi);


        }

        else if((ileri_tampon-eslesme)==0)
        {
            //sprintf(benzetme, "(0, %c)", ileri_tampon[0]);

            //eğer eşleşme bulunmadıysa karakter olduğu gibi kalıyor.
            lz77_cumlesi[q_lz77]=ileri_tampon[0];
            q_lz77++;

        }

        // ileri_tampon[strlen(ileri_tampon)]='\n';
        printf("[ILERI TAMPON]: %s", ileri_tampon);
        b.kay_miktar = kaydirma_uzunlugu(ileri_tampon-eslesme, eslesme_boyutu);
        printf("\n[BULUNAN BENZETME]: <%d,%d,[%c]>\n\n",ileri_tampon-eslesme, eslesme_boyutu, ileri_tampon[eslesme_boyutu]);
        fprintf(f,"%d. [BULUNAN BENZETME]: <%d,%d,[%c]>\n", i_kaydirma, ileri_tampon-eslesme, eslesme_boyutu, ileri_tampon[eslesme_boyutu]);

        ileri_tampon += eslesme_boyutu;
        //eşleşme boyutuna göre ileri tamponundan arama tamponuna karakter aktarımı yapılır.
        b.c = *ileri_tampon;
        i_kaydirma++;


        if(bel_say + 1 > kapasite)
        {
            kapasite = kapasite << 1;
            kodla = realloc(kodla,kapasite*sizeof(belirt));
        }
        kodla[bel_say++] = b ;
    }
    if(belirtec_say)
    {
        *belirtec_say = bel_say;
    }

    //printf("LZ77_cumlesi: %s\n",lz77_cumlesi);
    return kodla;
}

void frekanslar(char *str, FILE *f)
{
    //huffman kodlamayı gerçekleştirmek için ilk önce lzss metoduyla şifrelenen bloğun karakter frekansı bulunuyor.
    int sayac=0;
    int i;
    int frek[256] = {0};
    for(i = 0; str[i] != '\0'; i++)
    {
        frek[str[i]]++;
    }

    for(i = 0; i < 256; i++)
    {
        if(frek[i] != 0)
        {
            sayac++;
        }
    }

    int frekans[sayac];
    char arr[sayac];
    //karakter ve frekansları array'lere atılıyor.
    int j=0;
    for(i = 0; i < 256; i++)
    {
        if(frek[i] != 0)
        {

            frekans[j]=frek[i];
            arr[j]=i;
            j++;
        }
    }
    //karakter ve frekansları huffmanı çalıştırmak üzere kodlamaya gönderiliyor.
    huffmani_calistir(arr, frekans, sayac, str, f);

}
void dosya_deflate(FILE *f)
{
    printf("\t---- DEFLATE sifrelenmesi ----\n\n");
    int sayac=0;
    int bel_sayisi_lzss;
    FILE *fp;
    fp=fopen("kaynak.txt", "r");
    f =fopen("DEFLATEsonuc.txt", "w");
    if(fp==NULL)
    {
        printf("Dosya acilamadi!");
        return -1;
    }

    int blok_uzunlugu = 32 ;
    //BLOK UZUNLUĞU DEĞİŞTİRMEK İÇİN BURADA!
    char string[blok_uzunlugu];
    char buf[blok_uzunlugu];
    int r = 0 ;
    char stringler[100][blok_uzunlugu];
    int temp;
    char kodlanmis_string[blok_uzunlugu];


    fprintf(f, "-----------DEFLATE METODUN SIFRELENMESI -----------\n");
    //fprintf(f, "\n%d. blogun LZSS hali ", sayac+1);
    while(fgets(string, blok_uzunlugu, fp)!=NULL)
    {

        printf("SIFRELENECEK BLOK: %s", string);
        fprintf(f,"SIFRELENECEK BLOK: %s\n", string);
        fseek(f, 0, SEEK_END);

        int metin_boyutu=strlen(string);
        lzsskodlanmis(string, metin_boyutu, &bel_sayisi_lzss, f); //lzss kodlamaya blok gönderiliyor.
    }
    fprintf(f,"DEFLATE'den sonra boyut: %d byte\n", deflate_toplam/8);
    //DEFLATE metodunun sıkıştırma oranı belirtiliyor.
    fclose(f);
}

void lzsskodlanmis(char *metin, int limit, int *belirtec_say, FILE *f)
{
    int bel_say = 0 ;
    int kapasite = 1 << 3 ; //1 sayisini 3 bit sola kaydır
    belirt b; // Belirteç oluşturuyoruz.
    char *ileri_tampon;
    char *arama_tampon;
    int lzss_boyut;
    printf("\n");
    belirt *kodla = malloc(kapasite*sizeof(belirt));
    for(ileri_tampon = metin ; ileri_tampon < metin+limit ; ileri_tampon++)
    {

        arama_tampon = ileri_tampon - max_kaydirma_bit;

        if(arama_tampon< metin)
        {

            arama_tampon = metin;
        }


        int eslesme_boyutu = 0 ;
        char *eslesme = ileri_tampon;

        for(; arama_tampon < ileri_tampon ; arama_tampon++)
        {
            int uzunluk = eslesme_uzunlugu(arama_tampon,ileri_tampon,max_uzunluk_bit);

            if(uzunluk > eslesme_boyutu)
            {

                eslesme_boyutu = uzunluk-1;
                eslesme = arama_tampon;

            }
        }

        if(ileri_tampon + eslesme_boyutu >= metin + limit)
        {

            eslesme_boyutu = metin + limit-ileri_tampon-1; //eğer eşleşmenin son karekteri de dahil ise eşletmeyi kısalt.
        }

        int temp_boyut = eslesme_boyutu;
        char benzetme[10];
        char lzss_ekle[5];
        if((ileri_tampon-eslesme)!=0)
        {
            temp_boyut+=1;
            sprintf(benzetme, " %d,%d ", ileri_tampon-eslesme, temp_boyut);
            strcat(lzss_cumlesi,benzetme);
            q_lzss=strlen(lzss_cumlesi);
            lzss_boyut++;
            //pointerler oluşturuluyor.

        }

        else if((ileri_tampon-eslesme)==0)
        {
            sprintf(benzetme, "(0, %c)", ileri_tampon[0]);
            lzss_cumlesi[q_lzss]=ileri_tampon[0];
            q_lzss++;

        }
        printf("[ILERI TAMPON] %s - [BENZETME] %s\n", ileri_tampon, benzetme);

        b.kay_miktar = kaydirma_uzunlugu(ileri_tampon-eslesme, eslesme_boyutu);

        ileri_tampon += eslesme_boyutu;
        b.c = *ileri_tampon;


        if(bel_say + 1 > kapasite)
        {
            kapasite = kapasite << 1;
            kodla = realloc(kodla,kapasite*sizeof(belirt));
        }

        kodla[bel_say++] = b;

    }

    if(belirtec_say)
    {

        *belirtec_say = bel_say;
    }

    lzss_cumlesi[q_lzss]='\0';
    printf("LZSS'den sonra: %s\n",lzss_cumlesi);
    fprintf(f,"LZSS'den sonra: %s\n",lzss_cumlesi);
    fseek(f, 0, SEEK_END);
    frekanslar(lzss_cumlesi,f);
    //huffmana göndermek için bloğun lzss hali gönderiliyor.
    memset(lzss_cumlesi,0,sizeof(lzss_cumlesi));
    //LZSS'nin sifrelenmesinden sonra boşaltma işlemi.
    q_lzss=0;
    printf("\n");
}
dugum * yigin_sil()
{
    dugum * min_eleman,*son_eleman;
    int cocuk,simdi;
    min_eleman = yigin[1];
    son_eleman = yigin[yigin_size--];
    for( simdi = 1 ; simdi*2 <= yigin_size ; simdi = cocuk)
    {
        cocuk = simdi*2;
        if(cocuk!= yigin_size && yigin[cocuk+1]->frekansi < yigin[cocuk]->frekansi )
        {
            cocuk++;
        }
        if(son_eleman->frekansi > yigin[cocuk]->frekansi)
        {
            yigin[simdi] = yigin[cocuk];
        }
        else
        {
            break;
        }
    }
    yigin[simdi] = son_eleman;
    return min_eleman;
}

//Huffman Kodlama (DEFLATE)
void huffmani_calistir(char harfler[], int frekanslari[], int n, char orijinal_cumle[], FILE *f)
{
    yigin[0] = (dugum *)malloc(sizeof(dugum));
    yigin[0]->frekansi = 0;
    int i;
    for(i=0; i<n; i++)
    {
        dugum * temp = (dugum *) malloc(sizeof(dugum));
        temp -> harf = harfler[i];
        temp -> frekansi = frekanslari[i];
        temp -> sol = temp -> sag = NULL;
        eleman_ekle(temp);
    }
    if(n==1)
    {
        printf("KARAKTER: %c KOD: 0\n",harfler[0]);
        return 0;
    }
    for(i=0; i<n-1 ; i++)
    {
        dugum * sol = yigin_sil();
        dugum * sag = yigin_sil();
        dugum * temp = (dugum*) malloc(sizeof(dugum));
        temp -> harf = 0;
        temp -> sol = sol;
        temp -> sag = sag;
        temp -> frekansi = sol->frekansi + sag -> frekansi;
        eleman_ekle(temp);
    }
    dugum *agac = yigin_sil();
    char kod[10];
    kod[0] = '\0';
    printf("\nHUFFMAN KODLAMA: \n");
    fprintf(f,"\nHUFFMAN KODLAMA: \n");
    huffmani_yazdir(agac,kod, n);
    cumle_yaz(f);

}
void eleman_ekle(dugum * eleman)
{
    yigin_size++;
    yigin[yigin_size] = eleman;
    int simdi = yigin_size;
    while(yigin[simdi/2] -> frekansi > eleman -> frekansi)
    {
        yigin[simdi] = yigin[simdi/2];
        simdi/= 2;
    }
    yigin[simdi] = eleman;
}

void huffmani_yazdir(dugum *temp,char *kod, int n)
{
    if(temp->sol==NULL && temp->sag==NULL)
    {
        printf("KARAKTER [%c] KOD [%s]\n",temp->harf,kod);
        atama(temp->harf, kod, n);
        return;
    }

    int uzunluk = strlen(kod);

    char sol_kod[10],sag_kod[10];

    strcpy(sol_kod,kod);
    strcpy(sag_kod,kod);

    sol_kod[uzunluk] = '0';
    sol_kod[uzunluk+1] = '\0';

    sag_kod[uzunluk] = '1';
    sag_kod[uzunluk+1] = '\0';

    huffmani_yazdir(temp->sol,sol_kod, n);
    huffmani_yazdir(temp->sag,sag_kod, n);

}
char huff_harf[100];
char huff_frek[100][10];
int h=0;

void atama(char harf[], char kod[], int harf_sayisi)
{
    huff_harf[h]=harf;
    strcpy(huff_frek[h], kod);
    h++;
    //harf ve kodları array'lere atılıyor.
}
//huffman cümlesinin yazdırılıması.
void cumle_yaz(FILE *f)
{
    for(int j=0; j<h; j++)
    {
        fprintf(f, "KARAKTER: %c FREKANS: %s\n", huff_harf[j], huff_frek[j]);
    }
    fprintf(f, "\n");
    char huffman_cumlesi[100000];
    //printf("%d\n", h);
    for(int i = 0; i<strlen(lzss_cumlesi); i++)
    {
        for(int j=0; j<h; j++)
        {
            if(lzss_cumlesi[i]==huff_harf[j])
            {
                //printf("%c ", lzss_cumlesi[i]);
                //printf("%s ",huff_frek[j]);
                strcat(huffman_cumlesi, huff_frek[j]);
            }

        }
    }
    printf("\n");

    //boşaltma işlemleri
    h=0;
    memset(huff_harf,0,strlen(huff_harf));
    for(int v=0; v=h; v++)
    {
        memset(huff_frek[v],0,strlen(huff_frek[v]));
    }
    printf("[%s] HUFFMAN CUMLESI: %s\n", lzss_cumlesi,huffman_cumlesi);
    fprintf(f, "HUFFMAN CUMLESI: %s\n", huffman_cumlesi);
    deflate_toplam+=strlen(huffman_cumlesi);


    memset(huffman_cumlesi,0,strlen(huffman_cumlesi));
}

