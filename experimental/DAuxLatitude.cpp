/**
 * \file DAuxLatitude.cpp
 * \brief Implementation for the GeographicLib::experimental::DAuxLatitude
 * class.
 *
 * \note This is just sample code.  It is not part of GeographicLib itself.
 *
 * This file is an implementation of the methods described in
 * - C. F. F. Karney,
 *   On auxiliary latitudes,
 *   Technical Report, SRI International, December 2022.
 *   https://arxiv.org/abs/2212.05818
 * .
 * Copyright (c) Charles Karney (2022-2023) <charles@karney.com> and licensed
 * under the MIT/X11 License.  For more information, see
 * https://geographiclib.sourceforge.io/
 **********************************************************************/

#include "DAuxLatitude.hpp"
#include <iostream>

#if defined(_MSC_VER)
// Squelch warnings about constant conditional expressions
#  pragma warning (disable: 4127)
#endif

namespace GeographicLib {
namespace experimental {

  using namespace std;

  template<typename T>
  T DAuxLatitude<T>::DRectifying(const angle& phi1, const angle& phi2) const {
    // Stipulate that phi1 and phi2 are in [-90d, 90d]
    T x = phi1.radians(), y = phi2.radians();
    if (x == y) {
      if (1) {
        T d;
        angle mu1(aux::Rectifying(phi1, &d));
        T tphi1 = phi1.tan(), tmu1 = mu1.tan();
        return
          isfinite(tphi1) ? d * Math::sq(aux::sc(tphi1)/aux::sc(tmu1)) : 1/d;
      } else {
        angle phin(phi1.normalized());
        T d = Math::sq(phin.x()) + aux::_e2m1 * Math::sq(phin.y());
        return aux::_e2m1 / (d * sqrt(d)) / aux::RectifyingRadius(1, false);
      }
    } else if (x * y < 0)
      return (aux::Rectifying(phi2).radians() -
              aux::Rectifying(phi1).radians()) / (y - x);
    else {
      angle bet1(aux::Parametric(phi1)), bet2(aux::Parametric(phi2));
      T dEdbet = DE(bet1, bet2), dbetdphi = DParametric(phi1, phi2);
      return aux::_fm1 * dEdbet / aux::RectifyingRadius(1, false) * dbetdphi;
    }
  }

  template<typename T>
  T DAuxLatitude<T>::DParametric(const angle& phi1, const angle& phi2) const {
    T tx = phi1.tan(), ty = phi2.tan(), r;
    // DbetaDphi = Datan(fm1*tx, fm1*ty) * fm1 / Datan(tx, ty)
    // Datan(x, y) = 1/(1 + x^2),                       for x = y
    //             = (atan(y) - atan(x)) / (y-x),       for x*y < 0
    //             = atan( (y-x) / (1 + x*y) ) / (y-x), for x*y > 0
    if (!(tx * ty >= 0))        // This includes, e.g., tx = 0, ty = inf
      r = (atan(aux::_fm1 * ty) - atan(aux::_fm1 * tx)) /
        (atan(ty) - atan(tx));
    else if (tx == ty) {        // This includes the case tx = ty = inf
      tx *= tx;
      if (tx <= 1)
        r = aux::_fm1 * (1 + tx) / (1 + aux::_e2m1 * tx);
      else {
        tx = 1/tx;
        r = aux::_fm1 * (1 + tx) / (aux::_e2m1 + tx);
      }
    } else {
      if (tx * ty <= 1)
        r = atan2(aux::_fm1 * (ty - tx), 1 + aux::_e2m1 * tx * ty)
          / atan2(        ty - tx , 1 +         tx * ty);
      else {
        tx = 1/tx; ty = 1/ty;
        r = atan2(aux::_fm1 * (ty - tx), aux::_e2m1 + tx * ty)
          / atan2(        ty - tx ,   1   + tx * ty);
      }
    }
    return r;
  }

  template<typename T>
  T DAuxLatitude<T>::DE(const angle& X, const angle& Y) const {
    angle Xn(X.normalized()), Yn(Y.normalized());
    // We assume that X and Y are in [-90d, 90d] and have the same sign
    // If not we would include
    //    if (Xn.y() * Yn.y() < 0)
    //      return d != 0 ? (E(X) - E(Y)) / d : 1;

    // The general formula fails for x = y = 0d and x = y = 90d.  Probably this
    // is fixable (the formula works for other x = y.  But let's also stipulate
    // that x != y .

    // Make both positive, so we can do the swap a <-> b trick
    Xn.y() = fabs(Xn.y()); Yn.y() = fabs(Yn.y());
    T x = Xn.radians(), y = Yn.radians(), d = y - x,
      sx = Xn.y(), sy = Yn.y(), cx = Xn.x(), cy = Yn.x(),
      k2;
    // Switch prolate to oblate; we then can use the formulas for k2 < 0
    if (false && aux::_f < 0) {
      d = -d; swap(sx, cx); swap(sy, cy);
      k2 = aux::_e2;
    } else {
      k2 = -aux::_e12;
    }
    // See DLMF: Eqs (19.11.2) and (19.11.4) letting
    // theta -> x, phi -> -y, psi -> z
    //
    // (E(y) - E(x)) / d = E(z)/d - k2 * sin(x) * sin(y) * sin(z)/d
    //                   = (E(z)/sin(z) - k2 * sin(x) * sin(y)) * sin(z)/d
    // tan(z/2) = (sin(x)*Delta(y) - sin(y)*Delta(x)) / (cos(x) + cos(y))
    //          = d * Dsin(x,y) * (sin(x) + sin(y))/(cos(x) + cos(y)) /
    //             (sin(x)*Delta(y) + sin(y)*Delta(x))
    //          = t = d * Dt
    // Delta(x) = sqrt(1 - k2 * sin(x)^2)
    // sin(z) = 2*t/(1+t^2); cos(z) = (1-t^2)/(1+t^2)
    T Dt = Dsin(x, y) * (sx + sy) /
      ((cx + cy) * (sx * sqrt(1 - k2 * sy*sy) + sy * sqrt(1 - k2 * sx*sx))),
      t = d * Dt, Dsz = 2 * Dt / (1 + t*t),
      sz = d * Dsz, cz = (1 - t) * (1 + t) / (1 + t*t),
      sz2 = sz*sz, cz2 = cz*cz, dz2 = 1 - k2 * sz2,
      // E(z)/sin(z)
      Ezbsz = aux::RF(cz2, dz2, 1) - k2 * sz2 * aux::RD(cz2, dz2, 1) / 3;
    return (Ezbsz - k2 * sx * sy) * Dsz;
  }

  template<typename T>
  T DAuxLatitude<T>::Datanhee(T x, T y) const {
    // atan(e*sn(tphi))/e:
    //  Datan(e*sn(x),e*sn(y))*Dsn(x,y)/Datan(x,y)
    // asinh(e1*sn(fm1*tphi)):
    //  Dasinh(e1*sn(fm1*x)), e1*sn(fm1*y)) *
    // e1 * Dsn(fm1*x, fm1*y) *fm1 / (e * Datan(x,y))
    // = Dasinh(e1*sn(fm1*x)), e1*sn(fm1*y)) *
    //  Dsn(fm1*x, fm1*y) / Datan(x,y)
    return aux::_f < 0 ?
      Datan(aux::_e * aux::sn(x),
            aux::_e * aux::sn(y)) * Dsn(x, y) :
      Dasinh(aux::_e1 * aux::sn(aux::_fm1 * x),
             aux::_e1 * aux::sn(aux::_fm1 * y)) *
      Dsn(aux::_fm1 * x, aux::_fm1 * y);
  }

  template<typename T>
  T DAuxLatitude<T>::DIsometric(const angle& phi1, const angle& phi2) const {
    // psi = asinh(tan(phi)) - e^2 * atanhee(tan(phi))
    T tphi1 = phi1.tan(), tphi2 = phi2.tan();
    return isnan(tphi1) || isnan(tphi2) ? numeric_limits<T>::quiet_NaN() :
      (isinf(tphi1) || isinf(tphi2) ? numeric_limits<T>::infinity() :
       (Dasinh(tphi1, tphi2) - aux::_e2 * Datanhee(tphi1, tphi2)) /
       Datan(tphi1, tphi2));
  }

  template<typename T>
  T DAuxLatitude<T>::DConvert(int auxin, int auxout,
                              const angle& zeta1,  const angle& zeta2) const {
    int k = aux::ind(auxout, auxin);
    if (k < 0) return numeric_limits<T>::quiet_NaN();
    if (auxin == auxout) return 1;
    if ( isnan(aux::_c[aux::Lmax * (k + 1) - 1]) )
      aux::fillcoeff(auxin, auxout, k);
    angle zeta1n(zeta1.normalized()), zeta2n(zeta2.normalized());
    return 1 + DClenshaw(true, zeta2n.radians() - zeta1n.radians(),
                         zeta1n.y(), zeta1n.x(), zeta2n.y(), zeta2n.x(),
                         aux::_c + aux::Lmax * k, aux::Lmax);
  }

  template<typename T>
  T DAuxLatitude<T>::DClenshaw(bool sinp, T Delta,
                               T szet1, T czet1, T szet2, T czet2,
                               const T c[], int K) {
    // Evaluate
    // (Clenshaw(sinp, szet2, czet2, c, K) -
    //  Clenshaw(sinp, szet1, czet1, c, K)) / Delta
    // or
    // sum(c[k] * (sin( (2*k+2) * zet2) - sin( (2*k+2) * zet2)), i, 0, K-1)
    //   / Delta
    // (if !sinp, then change sin->cos here.)
    //
    // Delta is EITHER 1, giving the plain difference OR (zeta2 - zeta1) in
    // radians, giving the divided difference.  Other values will give
    // nonsense.
    //
    int k = K;
    // suffices a b denote [1,1], [2,1] elements of matrix/vector
    T D2 = Delta * Delta,
      czetp = czet2 * czet1 - szet2 * szet1,
      szetp = szet2 * czet1 + czet2 * szet1,
      czetm = czet2 * czet1 + szet2 * szet1,
      // sin(zetm) / Delta
      szetmd =  (Delta == 1 ? szet2 * czet1 - czet2 * szet1 :
                 (Delta != 0 ? sin(Delta) / Delta : 1)),
      Xa =  2 * czetp * czetm,
      Xb = -2 * szetp * szetmd,
      u0a = 0, u0b = 0, u1a = 0, u1b = 0; // accumulators for sum
    for (--k; k >= 0; --k) {
      // temporary T = X . U0 - U1 + c[k] * I
      T ta = Xa * u0a + D2 * Xb * u0b - u1a + c[k],
        tb = Xb * u0a +      Xa * u0b - u1b;
      // U1 = U0; U0 = T
      u1a = u0a; u0a = ta;
      u1b = u0b; u0b = tb;
    }
    // P = U0 . F[0] - U1 . F[-1]
    // if sinp:
    //   F[0] = [ sin(2*zet2) + sin(2*zet1),
    //           (sin(2*zet2) - sin(2*zet1)) / Delta]
    //        = 2 * [ szetp * czetm, czetp * szetmd ]
    //   F[-1] = [0, 0]
    // else:
    //   F[0] = [ cos(2*zet2) + cos(2*zet1),
    //           (cos(2*zet2) - cos(2*zet1)) / Delta]
    //        = 2 * [ czetp * czetm, -szetp * szetmd ]
    //   F[-1] = [2, 0]
    T F0a = (sinp ? szetp :  czetp) * czetm,
      F0b = (sinp ? czetp : -szetp) * szetmd,
      Fm1a = sinp ? 0 : 1;  // Fm1b = 0;
    // Don't both to compute sum...
    // divided difference (or difference if Delta == 1)
    return 2 * (F0a * u0b + F0b * u0a  - Fm1a * u1b);
  }

  /// \cond SKIP
  // Instantiate
  template class DAuxLatitude<Math::real>;
#if GEOGRAPHICLIB_PRECISION != 2
  template class DAuxLatitude<double>;
#endif
  /// \endcond

} // namespace experimental
} // namespace GeographicLib