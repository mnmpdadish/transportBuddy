//
//  main.cpp
//  transportBody3D
//  by Simon Verret, Universite de Sherbrooke
//
//  code partially adapted from afmCond.c
//  by Maxime Charlebois, Universite de Sherbrooke
//  under MIT license (september 2017)
//

#define _USE_MATH_DEFINES
#include <math.h>
#include <complex.h>
#include <sys/stat.h>
#include <stdio.h>

#include <stdlib.h>
#include <string.h>



// FUNCTIONS FOR READING FILES

void readDouble(FILE * file, char * name,  double * value) {
    rewind(file);
    char tempbuff[200];
    while(!feof(file))
    {
        if (fgets(tempbuff,200,file))
        {
            char tmpstr1[50];
            char tmpstr2[50];
            sscanf(tempbuff, "%49s %49s\n", tmpstr1, tmpstr2);
            if (strcmp(tmpstr1,name)==0) { *value = atof(tmpstr2); return;}
        }
    }
    printf("\ncannot find the %s parameter in 'model.dat'", name);
    exit(1);
}

void readInt(FILE * file, char * name,  int * value) {
    rewind(file);
    char tempbuff[200];
    while(!feof(file))
    {
        if (fgets(tempbuff,200,file))
        {
            char tmpstr1[50];
            char tmpstr2[50];
            sscanf(tempbuff, "%49s %49s\n", tmpstr1, tmpstr2);
            if (strcmp(tmpstr1,name)==0) { *value = atoi(tmpstr2); return;}
        }
    }
    printf("\ncannot find the %s parameter in 'model.dat'", name);
    exit(1);
}




//// MAIN

int main(int argc, const char * argv[]) {
    
    
    //// SETTING PARAMETERS
    
    
    int markiewicz = 1; //bool
    double t;
    double tp;
    double tpp;
    double tppp;
    double tz;
    
    double ETA;
    int nOmega;
    int nK;
    int nKz;
    int nMu; double muMin; double muMax;
    int nT; double Tmin; double Tmax;
    int logT=0; //bool
    double amplitudeCutoff;
    
    
    //// SETTING PARAMETERS
    FILE * file = fopen("model.dat", "rt");
    if(file == NULL) {printf("file %s not found", "model.dat"); exit(1);}
    printf("reading parameters from model.dat\n\n") ;
    
    readDouble(file, "t",               &t);
    readDouble(file, "tp",              &tp);
    readDouble(file, "tpp",             &tpp);
    readDouble(file, "tppp",            &tppp);
    readDouble(file, "tz",              &tz);
    
    readDouble(file, "muMin",           &muMin);
    readDouble(file, "muMax",           &muMax);
    readDouble(file, "Tmin",            &Tmin);
    readDouble(file, "Tmax",            &Tmax);
    readDouble(file, "ETA",             &ETA);
    readDouble(file, "amplitudeCutoff", &amplitudeCutoff);
    
    readInt(file, "nMu",    &nMu);
    readInt(file, "logT",   &logT); //to act as a bool
    readInt(file, "nT",     &nT);
    readInt(file, "nK",     &nK);
    readInt(file, "nKz",    &nKz);
    readInt(file, "nOmega", &nOmega);
    
    fclose(file);
    
    

    
    //// COMPUTING
    
    printf("transportBuddy starting\n\n");
    
    
    FILE *fileOut = fopen("transport_vs_T.dat","w");
    
    
    //// FOR A GIVEN TEMPERATURE
    double omega[nT][nOmega];
    double energyCutoff[nT];
    double dfermiDirac_dw[nT][nOmega];
    double dfermiDirac_dT[nT][nOmega];
    
    printf("%d \n",logT);
    int nn=0; for(nn=0; nn<nT; nn++)
    {
        double T = Tmin;
        if (Tmin != Tmax) {
            if(logT==1) T *= exp(nn*log(Tmax/Tmin)/(nT-1));
            else T += nn*(Tmax-Tmin)/(nT-1);
        }
        printf("%f ",T);
        double beta = 1./T;
        energyCutoff[nn] = 2.*2.*acosh(0.25*sqrt(beta/amplitudeCutoff)) /beta;
        
        int n=0; for(n=0; n<nOmega; n++)
        {
            omega[nn][n]= -energyCutoff[nn] + 2.*energyCutoff[nn]*n/(nOmega-1);
            double expBw = exp(beta*omega[nn][n]);
            dfermiDirac_dw[nn][n]= -beta*expBw/((expBw+1.)*(expBw+1.));
            dfermiDirac_dT[nn][n]= -beta*omega[nn][n] * dfermiDirac_dw[nn][n];
        }
    }
    
    
    //// ZERO TEMPERATURE RESULTS RECORDER
    double dos[nMu];
    double density0[nMu];
    double sigmaxx0[nMu];
    double sigmazz0[nMu];
    double sigmaxy0[nMu];
    
    //// TEMPERATURE DEPENDANT RESULTS RECORDER
    double Cv[nMu][nT];
    double density[nMu][nT];
    double sigma_xx[nMu][nT];
    double sigma_zz[nMu][nT];
    double sigma_xy[nMu][nT];
    double alpha_xx[nMu][nT];
    double alpha_zz[nMu][nT];
    double alpha_xy[nMu][nT];
    double beta_xx[nMu][nT];
    double beta_zz[nMu][nT];
    double beta_xy[nMu][nT];
    
    
    //// LOOP ON mu
    for(int iMu=0; iMu<nMu; iMu++)
    {
        
        double mu = muMin;
        if (muMin!=muMax) mu += iMu*(muMax-muMin)/(nMu-1);
        
        //// RESULTS
        dos[iMu] = 0.;
        density0[iMu] = 0.;
        sigmaxx0[iMu] = 0.;
        sigmazz0[iMu] = 0.;
        sigmaxy0[iMu] = 0.;
        int nn=0; for(nn=0; nn<nT; nn++)
        {
            Cv[iMu][nn]=0.;
            density[iMu][nn]=0.;
            sigma_xx[iMu][nn]=0.;
            sigma_zz[iMu][nn]=0.;
            sigma_xy[iMu][nn]=0.;
            alpha_xx[iMu][nn]=0.;
            alpha_zz[iMu][nn]=0.;
            alpha_xy[iMu][nn]=0.;
            beta_xx[iMu][nn]=0.;
            beta_zz[iMu][nn]=0.;
            beta_xy[iMu][nn]=0.;
        }
        
        //// INTEGRAL ON k
        for(int ii=0; ii<nK; ii++)
        {
            
            
            printf("muu= %i / %i, k = %i / %i \n",iMu,nMu,ii,nK);//fflush(stdout);
            
            double kx = M_PI*(-1.0 + ii*2.0/(nK));
            
            double coskx   = cos(kx);
            double cos2kx  = cos(2.*kx);
            double coskx_2 = cos(kx/2.);
            double sinkx   = sin(kx);
            double sin2kx  = sin(2.*kx);
            double sinkx_2 = sin(kx/2.);
            
            
            for(int jj=0; jj<nK; jj++)
            {
                double ky = M_PI*(-1.0 + jj*2.0/(nK));
                
                double cosky   = cos(ky);
                double cos2ky  = cos(2.*ky);
                double cosky_2 = cos(ky/2.);
                double sinky   = sin(ky);
                double sin2ky  = sin(2.*ky);
                double sinky_2 = sin(ky/2.);
                
                double epsilon_k   = -mu;
                epsilon_k         += -2.*t    * (coskx + cosky);
                epsilon_k         += -4.*tp   *  coskx*cosky;
                epsilon_k         += -2.*tpp  * (cos2kx + cos2ky);
                epsilon_k         += -4.*tppp * (cos2kx*cosky + coskx*cos2ky);
                double depsilon_dkx = 0;
                depsilon_dkx      +=  2.*t    *  sinkx;
                depsilon_dkx      +=  4.*tp   *  sinkx*cosky;
                depsilon_dkx      +=  4.*tpp  *  sin2kx;
                depsilon_dkx      +=     tppp * (8.*sin2kx*cosky + 4.*sinkx*cos2ky);
                double depsilon_dky = 0;
                depsilon_dky      +=  2.*t    *  sinky;
                depsilon_dky      +=  4.*tp   *  sinky*coskx;
                depsilon_dky      +=  4.*tpp  *  sin2ky;
                depsilon_dky      +=     tppp * (8.*sin2ky*coskx + 4.*sinky*cos2kx);
                double depsilon_dkx_dky = 0;
                depsilon_dkx_dky  += -4.*tp   *  sinkx*sinky;
                depsilon_dkx_dky  += -8.*tppp * (sin2kx*sinky + sinkx*sin2ky);
                double depsilon_dky_dky = 0;
                depsilon_dky_dky  +=  2.*t    *  cosky;
                depsilon_dky_dky  +=  4.*tp   *  cosky*coskx;
                depsilon_dky_dky  +=  8.*tpp  *  cos2ky;
                depsilon_dky_dky  +=     tppp * (16.*cos2ky*coskx + 8.*cosky*cos2kx);
                
                
                for(int kk=0; kk<nKz; kk++)
                {
                    
                    double kz = M_PI*(kk*2.0/(nKz)); // 0 to 2Pi assumes parity (period of Markie is 4Pi)
                    
                    double coskz   = cos(kz);
                    //double cos2kz  = cos(2.*kz);
                    double coskz_2 = cos(kz/2.);
                    //double sinkz   = sin(kz);
                    //double sin2kz  = sin(2.*kz);
                    double sinkz_2 = sin(kz/2.);
                    
                    double epsilonz_k         = -2.*tz*coskz;
                    double depsilonz_dkx      =  0.;
                    double depsilonz_dky      =  0.;
                    double depsilonz_dkx_dky  =  0.;
                    double depsilonz_dky_dky  =  0.;
                    double depsilonz_dkz      =  2.*tz*sinkx;
                    if (markiewicz) {
                        epsilonz_k             = -2.*tz*coskz_2;
                            epsilonz_k        *=  (coskx-cosky)*(coskx-cosky);
                            epsilonz_k        *=   coskx_2*cosky_2;
                        depsilonz_dkx          = -2.*tz*coskz_2;
                            depsilonz_dkx     *= -2*(coskx-cosky)*sinkx*coskx_2*cosky_2;
                            depsilonz_dkx     *=    (coskx-cosky)*(coskx-cosky)*sinkx_2*cosky_2 /2.;
                        depsilonz_dky          = -2*(coskx-cosky)*sinky*coskx_2*cosky_2;
                            depsilonz_dky     *=    (coskx-cosky)*(coskx-cosky)*coskx_2*sinky_2 /2.;
                        depsilonz_dkx_dky      = -2.*tz*coskz_2;
                            depsilonz_dkx_dky *=-2*(coskx+sinky)*sinkx*coskx_2*cosky_2;
                            depsilonz_dkx_dky *=  2*(coskx-cosky)*sinkx*coskx_2*sinky_2 /2.;
                            depsilonz_dkx_dky *= -2*(coskx-cosky)*sinky*sinkx_2*cosky_2 /2.;
                            depsilonz_dkx_dky *=   -(coskx-cosky)*(coskx-cosky)*sinkx_2*sinky_2 /4.;
                        depsilonz_dky_dky      = -2.*tz*coskz_2;
                            depsilonz_dky_dky *= -2*(coskx+sinky)*sinky*coskx_2*cosky_2;
                            depsilonz_dky_dky *= -2*(coskx-cosky)*cosky*coskx_2*cosky_2;
                            depsilonz_dky_dky *=  2*(coskx-cosky)*sinky*coskx_2*sinky_2 /2.;
                            depsilonz_dky_dky *= -2*(coskx-cosky)*sinky*coskx_2*sinky_2 /2.;
                            depsilonz_dky_dky *=    (coskx-cosky)*(coskx-cosky)*coskx_2*cosky_2 /4.;
                        depsilonz_dkz          = -2.*tz*sinkz_2;
                            depsilonz_dkz     *=  (coskx-cosky)*(coskx-cosky);
                            depsilonz_dkz     *=   coskx_2*cosky_2;
                    }

                    double ep_k        =  epsilon_k   +  epsilonz_k;
                    double dep_dkx     = depsilon_dkx + depsilonz_dkx;
                    double dep_dky     = depsilon_dky + depsilonz_dky;
                    double dep_dkz     = depsilonz_dkz;
                    double dep_dkx_dky = depsilon_dkx_dky + depsilonz_dkx_dky;
                    double dep_dky_dky = depsilon_dky_dky + depsilonz_dky_dky;
                    
                    
                    
                    double Gamma = ETA;  /// COULD BE MEAN-FREE PATH HERE
                    
                    double Ak0   = -(1./M_PI)*cimag(1.0/ (I*Gamma - ep_k));
                    double kernel_xx = dep_dkx*dep_dkx;
                    double kernel_zz = dep_dkz*dep_dkz;
                    double kernel_xy = -(2./3.)*(dep_dkx*(dep_dkx*dep_dky_dky - dep_dky*dep_dkx_dky));
                    
                    
                    dos[iMu]      += Ak0;
                    density0[iMu] += 1.0/(1.0+exp(1000*ep_k));
                    sigmaxx0[iMu] += kernel_xx*Ak0*Ak0;
                    sigmaxy0[iMu] += kernel_xy*Ak0*Ak0*Ak0;
                    sigmazz0[iMu] += kernel_zz*Ak0*Ak0;
                    
                    
                    //// LOOP ON TEMPERATURES
                    
                    int nn=0; for(nn=0; nn<nT; nn++)
                    {
                        double T = Tmin;
                        if (Tmin != Tmax) {
                            if(logT==1) T *= exp(nn*log(Tmax/Tmin)/(nT-1));
                            else T += nn*(Tmax-Tmin)/(nT-1);
                        }
                        double beta = 1./T;
                        
                        
                        density[iMu][nn]    += 1.0/(1.0+exp(beta*ep_k));

                        //// INTEGRAL IN ENERGY
                        int n=0; for(n=0; n<nOmega; n++)
                        {
                            double complex z = omega[nn][n] + ETA * I;
                            double A_k = -(1./M_PI)*cimag(1.0/ (z-ep_k) );

                            double CvKernel = omega[nn][n]*dfermiDirac_dT[nn][n]*A_k;
                            Cv[iMu][nn]         += CvKernel;

                            double frequencyKernel_xx = -dfermiDirac_dw[nn][n]*kernel_xx*A_k*A_k;
                            double frequencyKernel_zz = -dfermiDirac_dw[nn][n]*kernel_zz*A_k*A_k;
                            double frequencyKernel_xy = -dfermiDirac_dw[nn][n]*kernel_xy*A_k*A_k*A_k;
                            sigma_xx[iMu][nn] += frequencyKernel_xx;
                            sigma_zz[iMu][nn] += frequencyKernel_zz;
                            sigma_xy[iMu][nn] += frequencyKernel_xy;
                            alpha_xx[iMu][nn] += beta*omega[nn][n] * frequencyKernel_xx;
                            alpha_zz[iMu][nn] += beta*omega[nn][n] * frequencyKernel_zz;
                            alpha_xy[iMu][nn] += beta*omega[nn][n] * frequencyKernel_xy;
                            double omega2 = beta*beta* omega[nn][n] * omega[nn][n];
                            beta_xx[iMu][nn]  += omega2 * frequencyKernel_xx;
                            beta_zz[iMu][nn]  += omega2 * frequencyKernel_zz;
                            beta_xy[iMu][nn] += omega2 * frequencyKernel_xy;
                        }
                    }
                }
            }
        }
        
        double f0 = 2.0/nK/nK/nKz;
        printf("% 4.8f % 4.8f % 4.8f ", mu, 1.0-f0*density0[iMu], f0*dos[iMu]);
        printf("% 4.8f % 4.8f % 4.8f ", f0*sigmaxx0[iMu], f0*sigmaxy0[iMu], f0*sigmazz0[iMu]);
        printf("\n");
        fflush(stdout);
        
        
        
        //// FILE GROUPED BY T
        fprintf(fileOut, "           mu            p0           dos ");
        fprintf(fileOut, "     sigmaxx0      sigmaxy0      sigmazz0 ");
        fprintf(fileOut, "            T             p            Cv ");
        fprintf(fileOut, "      sigmaxx       sigmaxy       sigmazz ");
        fprintf(fileOut, "      alphaxx       alphaxy       alphazz ");
        fprintf(fileOut, "       betaxx        betaxy        betazz\n");
        
        //// LOOP ON TEMPERATURES
        nn=0; for(nn=0; nn<nT; nn++)
        {
            double T = Tmin;
            if (Tmin != Tmax) {
                if(logT==1) T *= exp(nn*log(Tmax/Tmin)/(nT-1));
                else T += nn*(Tmax-Tmin)/(nT-1);
            }
            double beta = 1./T;
            
            double f = (2.*energyCutoff[nn]) /(nOmega)*f0;
            
            fprintf(fileOut,"% 13f % 13f % 13f ", mu, 1.0-f0*density0[iMu], f0*dos[iMu]);
            fprintf(fileOut,"% 13f % 13f % 13f ", f0*sigmaxx0[iMu], f0*sigmaxy0[iMu], f0*sigmazz0[iMu]);
            
            fprintf(fileOut,"% 13f % 13f % 13f ", 1/beta, 1.0-f0*density[iMu][nn], f*Cv[iMu][nn]);
            fprintf(fileOut,"% 13f % 13f % 13f ", f*sigma_xx[iMu][nn], f*sigma_xy[iMu][nn], f*sigma_zz[iMu][nn] );
            fprintf(fileOut,"% 13f % 13f % 13f ", f*alpha_xx[iMu][nn], f*alpha_xy[iMu][nn], f*alpha_zz[iMu][nn] );
            fprintf(fileOut,"% 13f % 13f % 13f ", f*beta_xx[iMu][nn],  f*beta_xy[iMu][nn],  f*beta_zz[iMu][nn] );
            fprintf(fileOut, "\n");
        }
        fprintf(fileOut, "\n\n");
        fflush(fileOut);
        
        
    }
    fclose(fileOut);
    
    
    
    
    //// FILE GROUPED BY mu
    FILE *fileOutMu = fopen("transport_vs_mu.dat","w");
    
    nn=0; for(nn=0; nn<nT; nn++)
    {
        double T = Tmin;
        if (Tmin != Tmax) {
            if(logT==1) T *= exp(nn*log(Tmax/Tmin)/(nT-1));
            else T += nn*(Tmax-Tmin)/(nT-1);
        }
        double beta = 1./T;
        
        double f0 = 2.0/nK/nK/nKz;
        double f = (2.*energyCutoff[nn])/(nOmega)*f0;
        
        
        fprintf(fileOutMu, "           mu            p0           dos ");
        fprintf(fileOutMu, "     sigmaxx0      sigmaxy0      sigmazz0 ");
        fprintf(fileOutMu, "            T             p            Cv ");
        fprintf(fileOutMu, "      sigmaxx       sigmaxy       sigmazz ");
        fprintf(fileOutMu, "      alphaxx       alphaxy       alphazz ");
        fprintf(fileOutMu, "       betaxx        betaxy        betazz\n");
        
        for(int iMu=0; iMu<nMu; iMu++)
        {
            double mu = muMin + iMu*(muMax-muMin)/(nMu-1);
        
            fprintf(fileOutMu,"% 13f % 13f % 13f ", mu, 1.0-f0*density0[iMu], f0*dos[iMu]);
            fprintf(fileOutMu,"% 13f % 13f % 13f ", f0*sigmaxx0[iMu], f0*sigmaxy0[iMu], f0*sigmazz0[iMu]);
            
            fprintf(fileOutMu,"% 13f % 13f % 13f ", 1/beta, 1.0-f0*density[iMu][nn], f*Cv[iMu][nn]);
            fprintf(fileOutMu,"% 13f % 13f % 13f ", f*sigma_xx[iMu][nn], f*sigma_xy[iMu][nn], f*sigma_zz[iMu][nn] );
            fprintf(fileOutMu,"% 13f % 13f % 13f ", f*alpha_xx[iMu][nn], f*alpha_xy[iMu][nn], f*alpha_zz[iMu][nn] );
            fprintf(fileOutMu,"% 13f % 13f % 13f ", f*beta_xx[iMu][nn],  f*beta_xy[iMu][nn],  f*beta_zz[iMu][nn] );
            fprintf(fileOutMu, "\n");
        
        }
        fprintf(fileOutMu, "\n\n");
    }
    
    printf("\n transportBuddy over.\n");
    return 0;
}




