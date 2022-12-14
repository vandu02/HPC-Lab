/* Simple multigrid solver on unit square, using Jacobi smoother
 * (c) 2014 Philipp Neumann, TUM I-5
 */

#ifndef _ERROR_H_
#define _ERROR_H_

#include <cmath>
#include "VTKPlotter.h"
#include <iostream>
#include <cstdlib>
#include <omp.h>
#include <immintrin.h>

//#define NUM_THREADS 128

// Computes the L2-vector norm of the fine grid.
// We assume the solution to be u(x,y)=x*y on the unit square.
class alignas(32) ComputeError
{
public:
  ComputeError(const unsigned int nx, const unsigned int ny)
      : _error(new (std::align_val_t(32)) FLOAT[(nx + 2) * (ny + 2)]), _nx(nx), _ny(ny), _NUM_THREADS((nx < 16)? 1 : 128)
  {
    if (_error == NULL)
    {
      std::cout << "ERROR ComputeError(): _error==NULL!" << std::endl;
      exit(EXIT_FAILURE);
    }
  
    #pragma omp parallel for schedule(static) num_threads(_NUM_THREADS)
    for (unsigned int i = 0; i < (nx + 2) * (ny + 2); i++)
    {
      _error[i] = 0.0;
    }
    //
  }
  ~ComputeError() { delete[] _error; }

//
  void computePointwiseError(const FLOAT *const fineGrid)
  {
    _errorMax = 0.0;
    unsigned int pos;
   
    #pragma omp parallel for schedule(static) num_threads(_NUM_THREADS)
    for (unsigned int y = 1; y < _ny + 1; y++)
    {
      for (unsigned int x = 1; x < _nx + 1; x++)
      {
        pos = (y-1)*(_nx+2) + x + _nx+2;
        _error[pos] = fineGrid[pos] - (((FLOAT)x * y) / ((FLOAT)(_nx + 1) * (_ny + 1)));
        if (fabs(_error[pos]) > _errorMax)
        {
          _errorMax = fabs(_error[pos]);
        }
      }
    }
  }
//

  // returns the error
  FLOAT getMaxError() const { return _errorMax; }

  // plots the pointwise error and writes the result to the file error.vtk
  void plotPointwiseError()
  {
    const VTKPlotter vtkPlotter;
    vtkPlotter.writeFieldData(_error, _nx, _ny, "error.vtk", "error", true);
  }

private:
  FLOAT *_error;          // stores the pointwise error
  FLOAT _errorMax;        // computes max. pointwise error
  const unsigned int _nx; // number of inner grid points (x-direction)
  const unsigned int _ny; // number of inner grid points (y-direction)
  int _NUM_THREADS;
};

#endif // _ERROR_H_
