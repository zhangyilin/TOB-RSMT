#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bookshelf_IO.h"
#include "memAlloc.h"
#include "flute.h"

float HPwl();
float flutewl();

char benchmarkPath[BUFFERSIZE];
int main (int argc, char *argv[])
{
    char auxFile[BUFFERSIZE], placefile[BUFFERSIZE];

    if(argc != 4) {
        printf("Usage: %s <benchmark_dir> <aux_file> <placement_file>\n",
               argv[0]);
        printf("    <benchmark_dir> is the benchmark file directory.\n");
        printf("    <aux_file> is the bookshelf format auxiliary file");
        printf(" (assume in <benchmark_dir>).\n");
        printf("    <placement_file> is the placement file");
        printf(" (assume in current directory).\n");
        exit(1);
    }    

    strcpy(benchmarkPath, argv[1]);
    strcpy(auxFile, argv[2]);
    strcpy(placefile, argv[3]);

    readAuxFile(benchmarkPath, auxFile);
    createHash(benchmarkPath, nodesFile);
    readNodesFile(benchmarkPath, nodesFile);
    readNetsFile(benchmarkPath, netsFile);
    readPlFile(".", placefile);
    freeHash();

    readLUT();

    printf("Half-perimeter wirelength: %.2f\n", HPwl());
    printf("FLUTE wirelength         : %.2f\n", flutewl());
}

float HPwl()
{
    float totx, toty, xu, xl, yu, yl, xOffset, yOffset;
    int i, j, k;

    totx = 0.0; toty = 0.0;
    for (j=1; j<=numNets; j++) {
        xl = yl = 1e9;
        xu = yu = -1e9;
        for (k=netlistIndex[j]; k<netlistIndex[j+1]; k++) {
            i = netlist[k];
            xOffset = xPinOffset[k];
            yOffset = yPinOffset[k];

            if (xCellCoord[i]+xOffset < xl) xl = xCellCoord[i]+xOffset;
            if (xu < xCellCoord[i]+xOffset) xu = xCellCoord[i]+xOffset;
            if (yCellCoord[i]+yOffset < yl) yl = yCellCoord[i]+yOffset;
            if (yu < yCellCoord[i]+yOffset) yu = yCellCoord[i]+yOffset;
        }
        totx += xu - xl;
        toty += yu - yl;
    }
    
    return totx + toty;
}

float flutewl()
{
    Tree t;
    DTYPE totwl;
    DTYPE x[MAXD], y[MAXD];
    float xOffset, yOffset;
    int i, j, k, r, d;

#ifdef YILIN
    FILE *fp;
    FILE *fp_plot;
    char file_name[BUFFERSIZE], file_name_plot[BUFFERSIZE];
    strcpy(file_name, benchmarkPath);
    strcat(file_name, "/");
    strcpy(file_name_plot, file_name);
    strcat(file_name, "flute.branch");
    strcat(file_name_plot, "flute.gpl");
    if((fp=fopen(file_name, "wa"))==NULL) {
        printf("flute.c: Cannot open %s file", file_name);
    }    
    if((fp_plot=fopen(file_name_plot, "wa"))==NULL) {
        printf("flute.c: Cannot open %s file", file_name_plot);
    }    
#endif
    totwl = 0;
    for (j=1; j<=numNets; j++) {
        d = netlistIndex[j+1] - netlistIndex[j];
        k = netlistIndex[j]; 
        for (r=0; r<d; r++) {
            i = netlist[k+r];
            xOffset = xPinOffset[k+r];
            yOffset = yPinOffset[k+r];
            x[r] = (DTYPE) xCellCoord[i]+xOffset;
            y[r] = (DTYPE) yCellCoord[i]+yOffset;
        }
#if ROUTING==1
        t = flute(d, x, y, ACCURACY); totwl += t.length;


#ifdef YILIN
    fprintf (fp, "DegreeNumber : %d\n", t.deg);
    for (i=0; i<2*t.deg-2; i++){
//        printf ("%dth branch's x at %d, y at %d, neighbor is %d\n", i, t.branch[i].x, t.branch[i].y, t.branch[i].n);
        fprintf (fp, "%-2d  %4d    %4d    %d\n", i, t.branch[i].x, t.branch[i].y, t.branch[i].n);
        fprintf (fp_plot, "%-2d  %4d    %4d    %d\n", t.branch[i].x, t.branch[i].y, t.branch[t.branch[i].n].x, t.branch[t.branch[i].n].y);
    }
#endif

#else        
        totwl += flute_wl(d, x, y, ACCURACY);
#endif        
    }
#ifdef YILIN
    fclose(fp);
    fclose(fp_plot);
#endif
    
    return (float) totwl;
}    