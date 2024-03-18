/* ipDFT PMU hook.
 *
 * Author: Andres Acosta <andres.acosta@eonerc.rwth-aachen.de>
 * SPDX-FileCopyrightText: 2014-2023 Institute for Automation of Complex Power Systems, RWTH Aachen University
 * SPDX-License-Identifier: Apache-2.0
 */

#include <villas/hooks/pmu.hpp>

namespace villas {
namespace node {

class TruncatedIpDftPmuHook : public PmuHook {

protected:
  std::complex<double> omega;
  std::vector<std::vector<double>> twf_dft_r;
  std::vector<std::vector<double>> twf_dft_i;

  unsigned P;
  unsigned startBin;

  unsigned frequencyCount; // Number of requency bins that are calculated
  double estimationRange;  // The range around nominalFreq used for estimation

  struct Complex {
    double r;
    double i;
    double mag;
    double ph;
    double f;
  };

  struct PhasorReIm {
    double r;
    double i;
  };

  std::vector<Complex> Xk;

public:
  TruncatedIpDftPmuHook(Path *p, Node *n, int fl, int prio, bool en = true)
      : PmuHook(p, n, fl, prio, en), P(1), frequencyCount(0), estimationRange(0)

  {}

  // PhasorReIm hann_frt(double k, unsigned M) {
  //   PhasorReIm val;
  //   double m = M_PI / M;
  //   double a = 0.5 * sin(M_PI * k) / sin(k * M);
  //   double b = 0.25 * sin(M_PI * (k+1)) / sin((k+1) * m);
  //   double c = 0.25 * sin(M_PI * (k-1)) / sin((k-1) * m);

  //   val.r = cos(k*(M-1)*m) * a - cos((k+1)*(M-1)*m) * b - cos((k-1)*(M-1)*m) * c;
  //   val.i = -sin(k*(M-1)*m) * a + sin((k+1)*(M-1)*m) * b + sin((k-1)*(M-1)*m) * c;
  //   return val;
  // }

  void prepare() {
    PmuHook::prepare();

    // This is assuming that the window size (in number of samples) covers a period of the signal
    // const double frequencyResolution = nominalFreq;

    // Number of samples per frame consider minimum Fres<f0/2. Samples per reported phasor
    // windowSize = sampleRate / frequencyResolution;

    // Time window per frame (ms)
    // const double Tw = 1000 / phasorRate;

    // Number of frequency bins given the frequency resolution: Fres=Nr=Fs/Nc
    if (phasorRate < 5) {
      frequencyCount = 16;
    } else {
      frequencyCount = (nominalFreq / phasorRate) + 2;
    }

    if (frequencyCount%2 == 1) {
      frequencyCount = frequencyCount + 1;
    }

    // Initialize matrix of dft coeffients
    startBin = (unsigned)floor(nominalFreq/phasorRate - frequencyCount/2);
    // unsigned endBin = startBin + frequencyCount;

    twf_dft_r.clear();
    twf_dft_i.clear();
    // twf_dft_r.reserve(windowSize*frequencyCount);
    // twf_dft_i.reserve(windowSize*frequencyCount);
    for (unsigned k = 0; k < frequencyCount; k++) {
      twf_dft_r.emplace_back(windowSize, 0.0);
      twf_dft_i.emplace_back(windowSize, 0.0);
    }

    const double dw = 2* M_PI / frequencyCount;

    // twiddle factor for truncated DFT
    for (unsigned k = 0; k < frequencyCount; k++) {
      for (unsigned n = 0; n < windowSize; n++) {
        twf_dft_r[k][n] = cos(n*(k+startBin+1)*dw);
        twf_dft_i[k][n] = sin(n*(k+startBin+1)*dw);
      }
      Xk.push_back({0., 0., 0., 0., 0.});
    }

  }

  void parse(json_t *json) {
    PmuHook::parse(json);
    int ret;

    json_error_t err;

    assert(state != State::STARTED);

    Hook::parse(json);

    ret = json_unpack_ex(json, &err, 0, "{ s?: F}", "estimation_range",
                         &estimationRange);

    if (ret)
      throw ConfigError(json, err, "node-config-hook-ip-dft-pmu");

    if (estimationRange <= 0)
      throw ConfigError(
          json, "node-config-hook-ip-dft-pmu-estimation_range",
          "Estimation range cannot be less or equal than 0 tried to set {}",
          estimationRange);
  }

  PmuHook::Phasor estimatePhasor(dsp::CosineWindow<double> *window,
                                 PmuHook::Phasor lastPhasor) {
    PmuHook::Phasor estimatedPhasor = {0};

    const double B = 0.5*frequencyCount; // Integration of wh over the whole window
    const double b = 1/B;

    int km = 0;

    for (unsigned n = 0; n < windowSize; n++) {
      for (unsigned k = 0; k < frequencyCount; k++) {
        // Real part of X[k]
        Xk[k].r += (*window).val(n) * twf_dft_r[k][n];
        // Imaginary part of X[k]
        Xk[k].i -= (*window).val(n) * twf_dft_i[k][n];
      }
    }
    for (unsigned k = 0; k < frequencyCount; k++) {
      Xk[k].r *= b;
      Xk[k].i *= b;

      Xk[k].mag = pow(Xk[k].r*Xk[k].r + Xk[k].i*Xk[k].i, 0.5);

      if (Xk[k].mag > Xk[km].mag) {
        km = k;
      }
    }

    std::vector<Complex> wXk = Xk;

    // Windowing in Frequency Domain
    for (unsigned k = 1; k < frequencyCount-1; k++) {
      wXk[k].r = -0.25*Xk[k-1].r + 0.50*Xk[k].r - 0.25*Xk[k+1].r;
      wXk[k].i = -0.25*Xk[k-1].i + 0.50*Xk[k].i - 0.25*Xk[k+1].i;
      wXk[k].mag = pow(wXk[k].r*wXk[k].r + wXk[k].i*wXk[k].i, 0.5);
    }

    Xk = wXk;

    int epsil = (Xk[km+1].mag > Xk[km-1].mag) ? 1 : -1;

    double alpha = std::abs(Xk[km].mag / Xk[km+epsil].mag);
    double delta = epsil * ((2 - alpha) / (1 + alpha));
    double a;
    if (delta < 0.00001) {
      a = 2.0;
    } else {
      a = 2 * M_PI * delta * (1 - pow(delta, 2)) / sin(M_PI * delta);
    }

    estimatedPhasor.frequency=(km + 1 + startBin + delta) * phasorRate;
    estimatedPhasor.amplitude = Xk[km].mag * a;
    estimatedPhasor.phase = atan2(Xk[km].i , Xk[km].r) - M_PI*(delta);
    estimatedPhasor.rocof = 0;

    // i_ipDFT starts here

    /*i_ipDFT works well for higher reporting rates i.e. also Nr=25, but
    e_ipDFT outperforms i_ipDFT for lower reporting rates (e.g. Nr=10)
    i_ipDFT (variables definition) starts here*/
    // PhasorReIm phasor;

    // if (phasorRate > 10) {
    //   phasor.r = Xk[km].r;
    //   phasor.i = Xk[km].i;
    //   PhasorReIm phasor_epsil = phasor;
    //   PhasorReIm Xneg = phasor;
    //   PhasorReIm Xneg_epsil = phasor;
    //   phasor_epsil.r = Xk[km+epsil].r;
    //   phasor_epsil.i = Xk[km+epsil].i;
    //   double delta_old = delta;

    //   // i_ipDFT
    //   for (unsigned q = 0; q < P; q++) {
    //     PhasorReIm hann_ft = hann_frt(delta + 2*(km+startBin-1), windowSize);
    //     Xneg.r = 0.5*(phasor.r*hann_ft.r + phasor.i*hann_ft.i)*b;
    //     Xneg.i = 0.5*(phasor.r*hann_ft.i - phasor.i*hann_ft.r)*b;

    //     hann_ft = hann_frt(delta + epsil + 2*(km+startBin-1), windowSize);
    //     Xneg_epsil.r = 0.5*(phasor.r*hann_ft.r + phasor.i*hann_ft.i)*b;
    //     Xneg_epsil.i = 0.5*(phasor.r*hann_ft.i - phasor.i*hann_ft.r)*b;
    //     phasor.r = phasor.r-Xneg.r;
    //     phasor.i = phasor.i-Xneg.i;

    //     phasor_epsil.r = phasor_epsil.r - Xneg_epsil.r;
    //     phasor_epsil.i = phasor_epsil.i - Xneg_epsil.i;

    //     Xk[km].mag = pow(pow(phasor.r, 2) + pow(phasor.i, 2), 0.5);
    //     Xk[km+epsil].mag = pow(pow(phasor_epsil.r, 2) + pow(phasor_epsil.i, 2), 0.5);

    //     alpha = abs(Xk[km].mag / Xk[km+epsil].mag);
    //     delta = epsil*(2-alpha) / (1+alpha);

    //     if (abs(delta - delta_old)==0){
    //       q = P+1;
    //     }
    //   }
    // } else {

    //   // end of i_ipDFT

    //   /*e_ipDFT
    //   e_ipDFT is very good from Nr=10 and Fs=10k; Not good otherwise.
    //   Nonetheless, for Nr>=10 it presents better precision than i_ipDFT
    //   */

    //   phasor.r = Xk[km].r;
    //   phasor.i = Xk[km].i;

    //   PhasorReIm v_ipdft;
    //   v_ipdft.r = a*(Xk[km].r*cos(M_PI*delta)+Xk[km].i*sin(M_PI*delta));
    //   v_ipdft.i = a*(-Xk[km].r*sin(M_PI*delta)+Xk[km].i*cos(M_PI*delta));

    //   // This part is iterated P times
    //   // variables allocation
    //   PhasorReIm v1;
    //   v1.r = v_ipdft.r;
    //   v1.i = v_ipdft.i;
    //   double e_delta_corr = delta;

    //   std::vector<PhasorReIm> e_ipdft_3max;

    //   for (unsigned k = 0; k < 3; k++) {
    //     e_ipdft_3max.push_back({0., 0.});
    //     e_ipdft_3max[k] = phasor;
    //   }

    //   e_ipdft_3max[0].r = Xk[km-1].r;
    //   e_ipdft_3max[0].i = Xk[km-1].i;
    //   e_ipdft_3max[1].r = Xk[km].r;
    //   e_ipdft_3max[1].i = Xk[km].i;
    //   e_ipdft_3max[2].r = Xk[km+1].r;
    //   e_ipdft_3max[2].i = Xk[km+1].i;

    //   std::vector<double> e_ipdft_3mag;
    //   for (unsigned k = 0; k < 3; k++) {
    //     e_ipdft_3mag.push_back(0.);
    //     e_ipdft_3mag[k] = phasor.r;
    //   }

    //   PhasorReIm v_e_ipdft_max = phasor;
    //   double e_a = phasor.r;
    //   PhasorReIm hann_ft = phasor;

    //   std::vector<PhasorReIm> e_ipdft_3max_new;
    //   for (unsigned k = 0; k < 3; k++) {
    //     e_ipdft_3max_new.push_back({0., 0.});
    //     e_ipdft_3max_new[k] = phasor;
    //   }

    //   if (abs(delta) > 0) {
    //     for (unsigned q = 0; q < P; q++) {
    //       for (unsigned j = 0; j < 3; j++) {
    //         hann_ft = hann_frt(j-2+e_delta_corr+2*(km+startBin), windowSize);

    //         e_ipdft_3max_new[j].r = e_ipdft_3max[j].r-(v1.r*hann_ft.r + v1.i*hann_ft.i)*b;
    //         e_ipdft_3max_new[j].i = e_ipdft_3max[j].i+(v1.i*hann_ft.r - v1.r*hann_ft.i)*b;
    //         e_ipdft_3mag[j] = pow(pow(e_ipdft_3max_new[j].r, 2) + pow(e_ipdft_3max_new[j].i, 2), 0.5);
    //       }

    //       // interpolating the three bins to get the fractional correction term "delta_corr"
    //       e_delta_corr = 2*epsil*(abs(e_ipdft_3mag[2]-e_ipdft_3mag[0]))/(e_ipdft_3mag[1]*2 + e_ipdft_3mag[0]+ e_ipdft_3mag[2]);

    //       v_e_ipdft_max = e_ipdft_3max_new[1];

    //       if (abs(e_delta_corr)<0.00001) {
    //         e_delta_corr = 0;
    //         v1.r = v_e_ipdft_max.r;
    //         v1.i = v_e_ipdft_max.i;
    //         q = P+1;
    //       } else {
    //         e_a = (1-pow(e_delta_corr, 2))*abs((M_PI*e_delta_corr)/sin(M_PI*e_delta_corr));
    //         v1.r = e_a*(v_e_ipdft_max.r*cos(M_PI*e_delta_corr)+v_e_ipdft_max.i*sin(M_PI*e_delta_corr));
    //         v1.i = e_a*(-v_e_ipdft_max.r*sin(M_PI*e_delta_corr)+v_e_ipdft_max.i*cos(M_PI*e_delta_corr));
    //       }

    //     }
    //     phasor = v1;
    //     delta = e_delta_corr;
    //   }

    // }
    // end e_ipDFT

    // double f = 0;
    // double fold = f;
    // Xk[km].f = (km + 1 + startBin + delta) * phasorRate;
    // if (fold > 0) {

    // }

    if (lastPhasor.frequency !=
        0) // Check if we already calculated a phasor before
      estimatedPhasor.valid = Status::VALID;

    return estimatedPhasor;
  }
};

// Register hook
static char n[] = "truncated-ip-dft-pmu";
static char d[] = "This hook calculates a phasor based on truncated ipDFT";
static HookPlugin<TruncatedIpDftPmuHook, n, d,
                  (int)Hook::Flags::NODE_READ | (int)Hook::Flags::NODE_WRITE |
                      (int)Hook::Flags::PATH>
    p;

} // namespace node
} // namespace villas
