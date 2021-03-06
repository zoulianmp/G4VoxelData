//////////////////////////////////////////////////////////////////////////
// G4VoxelData
// ===========
// A general interface for loading voxelised data as geometry in GEANT4.
//
// Author:  Christopher M Poole <mail@christopherpoole.net>
// Source:  http://github.com/christopherpoole/G4VoxelData
//
// License & Copyright
// ===================
// 
// Copyright 2013 Christopher M Poole <mail@christopherpoole.net>
// 
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//////////////////////////////////////////////////////////////////////////


#ifndef G4VOXELARRAY_H
#define G4VOXELARRAY_H

// G4VOXELDATA //
#include "G4VoxelData.hh"

// STL //
#include <vector>
#include "stdint.h"

// GEANT4 //
#include "globals.hh"
#include "G4ThreeVector.hh"


template <typename T>
class G4VoxelArrayBase {
  public:
    G4VoxelArrayBase() {
    };
  
    void Init() {
        this->cropped_shape = this->shape;
        this->order = COLUMN_MAJOR;

        // x-direction
        this->crop_limits.push_back(0);
        this->crop_limits.push_back(this->shape[0]);
        // y-direction
        this->crop_limits.push_back(0);
        this->crop_limits.push_back(this->shape[1]);
        // z-direction
        this->crop_limits.push_back(0);
        this->crop_limits.push_back(this->shape[2]);

        this->cropped = false;

        this->merged_shape = this->shape;
        this->merge_size.push_back(1);  // x-direction
        this->merge_size.push_back(1);  // y-direction
        this->merge_size.push_back(1);  // z-direction

        this->merged = false;
    };
 
    void Init(G4VoxelData* data) {
        this->data = data;
        this->shape = data->shape;

        Init();
        
        this->length = data->length;
        this->ndims = data->ndims;
        this->spacing = data->spacing;
        this->order = data->order;
    };

    G4VoxelData* GetData() {
        return this->data;
    };

    G4ThreeVector GetVoxelSize() {
        if (this->merged) {
            return G4ThreeVector(merge_size[0]*spacing[0]/2.,
                                 merge_size[1]*spacing[1]/2.,
                                 merge_size[2]*spacing[2]/2.);
        }

        return G4ThreeVector(spacing[0]/2., spacing[1]/2., spacing[2]/2.);
    };

    G4ThreeVector GetVolumeShape() {
        if (this->merged) {
            return G4ThreeVector(merged_shape[0], merged_shape[1], merged_shape[2]);
        }
        return G4ThreeVector(cropped_shape[0], cropped_shape[1], cropped_shape[2]);
    };

    G4ThreeVector GetOrigin() {
        return G4ThreeVector(data->origin[0], data->origin[1], data->origin[2]);
    };

    G4double GetOriginX() {
        return GetOrigin()[0];
    };

    G4double GetOriginY() {
        return GetOrigin()[1];
    };

    G4double GetOriginZ() {
        return GetOrigin()[2];
    };

    double GetPosition(unsigned int index, unsigned int shape, double spacing) {
        return (index * spacing) - (shape/2. * spacing);
    };

    std::vector<double> GetPosition(std::vector<unsigned int> indices) {
        std::vector<double> position;

        for (unsigned int i=0; i<indices.size(); i++) {
            double pos = GetPosition(indices[i], this->shape[i], (double) this->spacing[i]);
            position.push_back(pos);
        }

        return position;
    };

    std::vector<double> GetPosition(unsigned int x) {
        std::vector<unsigned int> indices;
        indices.push_back(x);

        return GetPosition(indices);
    };

    std::vector<double> GetPosition(unsigned int x, unsigned int y) {
        std::vector<unsigned int> indices;
        indices.push_back(x);
        indices.push_back(y);

        return GetPosition(indices);
    };
    
    std::vector<double> GetPosition(unsigned int x, unsigned int y, unsigned int z) {
        std::vector<unsigned int> indices;
        indices.push_back(x);
        indices.push_back(y);
        indices.push_back(z);

        return GetPosition(indices);
    };

    G4double GetPositionX(unsigned int x) {
        return GetPosition(x, shape[0], spacing[0]);
    };

    G4double GetPositionY(unsigned int y) {
        return GetPosition(y, shape[1], spacing[1]);
    };

    G4double GetPositionZ(unsigned int z) {
        return GetPosition(z, shape[2], spacing[2]);
    };

    G4ThreeVector GetPosition(G4ThreeVector indices) {
        std::vector<double> pos = GetPosition(
            (unsigned int) indices.x(),
            (unsigned int) indices.y(),
            (unsigned int) indices.z());
        return G4ThreeVector(pos[0], pos[1], pos[2]); 
    };

    unsigned int GetIndex(unsigned int index) {
        return index;
    };

    unsigned int GetIndex(std::vector<unsigned int> indices,
            std::vector<unsigned int> shape) {
        unsigned int index;

        if (this->order == ROW_MAJOR) {
            /* index = x + (shape[0] * y) + (shape[0] * shape[1] * z) */
            
            index = indices.front();
            for (unsigned int i=1; i<indices.size(); i++) {
                unsigned int offset = indices[i];
                for (unsigned int j=0; j<i; j++) {
                    offset *= shape[j];
                }
                index += offset;
            }
        } else if (this->order == COLUMN_MAJOR) {
            /* index = z + (shape[2] * y) + (shape[2] * shape[1] * x) */

            index = indices.back();
            for (unsigned int i=indices.size()-2; i<indices.size(); i--) {
                unsigned int offset = indices[i];
                for (unsigned int j=indices.size()-1; j>=i+1 && j<indices.size(); j--) {
                    offset *= shape[j];
                }
               index += offset; 
            }
        }
        return index;
    };

    unsigned int GetIndex(std::vector<unsigned int> indices) {
        return GetIndex(indices, this->shape);
    };
   
    unsigned int GetIndex(unsigned int x, unsigned int y,
            std::vector<unsigned int> shape) {
        std::vector<unsigned int> indices;
        indices.push_back(x);
        indices.push_back(y);

        return GetIndex(indices, shape);
    };

    unsigned int GetIndex(unsigned int x, unsigned int y) {
        return GetIndex(x, y, this->shape);
    };

    unsigned int GetIndex(unsigned int x, unsigned int y, unsigned int z,
            std::vector<unsigned int> shape) {
        std::vector<unsigned int> indices;
        indices.push_back(x);
        indices.push_back(y);
        indices.push_back(z);

        return GetIndex(indices, shape);
    };

    unsigned int GetIndex(unsigned int x, unsigned int y, unsigned int z) {
        return GetIndex(x, y, z, this->shape);
    };

    unsigned int GetIndex(G4ThreeVector position) {
        return GetIndex(position.x(), position.y(), position.z());
    };

    std::vector<unsigned int> UnpackIndices(unsigned int index) {
        unsigned int x, y, z, sub_index;

        if (this->order == ROW_MAJOR) {
            /* index = x + (shape[0] * y) + (shape[0] * shape[1] * z) */
            x = index / (this->shape[1] * this->shape[2]);

            sub_index = index % (this->shape[1] * this->shape[2]);
            y = sub_index / this->shape[1];
            z = sub_index % this->shape[1];
        } else if (this->order == COLUMN_MAJOR) {
            /* index = z + (shape[2] * y) + (shape[2] * shape[1] * x) */
            z = index / (this->shape[0] * this->shape[1]);

            sub_index = index % (this->shape[0] * this->shape[1]);
            y = sub_index / this->shape[0];
            x = sub_index % this->shape[0];
        }

        std::vector<unsigned int> indices;
        indices.push_back(x);
        indices.push_back(y);
        indices.push_back(z);

        return indices;
    };

    void CropAxis(unsigned int imin, unsigned int imax, unsigned int axis) {
        cropped_shape[axis] = imax - imin;

        crop_limits[2*axis] = imin;
        crop_limits[(2*axis) + 1] = imax;

        cropped = true;
    };

    void CropX(unsigned int xmin, unsigned int xmax) {
        CropAxis(xmin, xmax, 0);
    };

    void CropY(unsigned int ymin, unsigned int ymax) {
        CropAxis(ymin, ymax, 1);
    };

    void CropZ(unsigned int zmin, unsigned int zmax) {
        CropAxis(zmin, zmax, 2);
    };

    void Crop(unsigned int xmin, unsigned int xmax,
              unsigned int ymin, unsigned int ymax,
              unsigned int zmin, unsigned int zmax) {
        CropX(xmin, xmax);
        CropY(ymin, ymax);
        CropZ(zmin, zmax);
    };

    void Crop(bool cropped) {
       this->cropped = cropped; 
    };

    bool IsCropped() {
        return this->cropped;
    };

    void ClearCrop() {
        CropX(0, shape[0]); 
        CropY(0, shape[1]); 
        CropZ(0, shape[2]); 
        
        cropped = false;
    };
    
    std::vector<unsigned int> GetCropLimit() {
        return this->crop_limits;
    }; 

    void MergeAxis(unsigned int size, unsigned int axis) {
        if (cropped_shape[axis] % size != 0) {

        }

        merged_shape[axis] = cropped_shape[axis] / size;
        merge_size[axis] = size;

        merged = true;
    };

    void MergeX(unsigned int size) {
        MergeAxis(size, 0); 
    };

    void MergeY(unsigned int size) {
        MergeAxis(size, 1); 

    };

    void MergeZ(unsigned int size) {
        MergeAxis(size, 2); 
    };

    void Merge(unsigned int size_x, unsigned int size_y,
               unsigned int size_z) {
        MergeX(size_x);
        MergeY(size_y);
        MergeZ(size_z);
    };

    bool IsMerged() {
        return this->merged;
    };

    void ClearMerge() {
        MergeX(1); 
        MergeY(1); 
        MergeZ(1); 

        merged = false;
    };

    std::vector<unsigned int> GetMergeSize() {
        return this->merge_size;
    };

    unsigned int GetLength() {
        return this->length / sizeof(T);
    };

    unsigned int GetDimensions() {
        return this->ndims;
    };

    void SetDimensions(unsigned int ndims) {
        this->ndims = ndims;
    };

    std::vector<unsigned int> GetShape() {
        if (this->merged) {
            return this->merged_shape; 
        }
        return this->cropped_shape;
    };

    unsigned int GetShapeX() {
        return GetShape()[0];
    };

    unsigned int GetShapeY() {
        return GetShape()[1];
    };

    unsigned int GetShapeZ() {
        return GetShape()[2];
    };

    void SetShape(std::vector<unsigned int> shape) {
        this->shape = shape;
    }

    void SetShape(G4ThreeVector shape) {
        std::vector<double> s;
        s.push_back(shape.x());
        s.push_back(shape.y());
        s.push_back(shape.z());

        SetShape(s);
    };

    std::vector<double> GetSpacing() {
        if (this->merged) {
            std::vector<double> spacing;
            for (unsigned int i=0; i<this->ndims; i++) {
                spacing.push_back(this->spacing[i] * this->merge_size[i]);
            }
            return spacing;
        }
        return this->spacing;
    };

    unsigned int GetSpacingX() {
        return GetSpacing()[0];
    };

    unsigned int GetSpacingY() {
        return GetSpacing()[1];
    };

    unsigned int GetSpacingZ() {
        return GetSpacing()[2];
    };

    void SetSpacing(std::vector<double> spacing) {
        this->spacing = spacing;
    };

    void SetSpacing(G4ThreeVector spacing) {
        std::vector<double> s;
        s.push_back(spacing.x());
        s.push_back(spacing.y());
        s.push_back(spacing.z());

        SetSpacing(s);
    };

  protected:
    G4VoxelData* data;

    unsigned int length;
    unsigned int ndims;
    
    Order order;

    std::vector<unsigned int> shape;
    std::vector<double> spacing;

    bool cropped;
    std::vector<unsigned int> crop_limits;
    std::vector<unsigned int> cropped_shape;

    bool merged;
    std::vector<unsigned int> merge_size;
    std::vector<unsigned int> merged_shape;
};


template <typename T>
class G4VoxelArray : public G4VoxelArrayBase<T> {
  public:
    using G4VoxelArrayBase<T>::Init;

    G4VoxelArray() {
    };

    G4VoxelArray(G4VoxelData* data) {
        SetData(data);
    };

    G4VoxelArray(std::vector<unsigned int> shape, std::vector<double> spacing) {
        G4VoxelData* data = new G4VoxelData(shape, spacing, sizeof(T));
        Init(data);
        
        this->array = reinterpret_cast<std::vector<T>*>(data->array);
    }

    G4VoxelArray(G4ThreeVector shape, G4ThreeVector spacing) {
        std::vector<unsigned int> sh;
        sh.push_back(shape.x());
        sh.push_back(shape.y());
        sh.push_back(shape.z());

        std::vector<double> sp;
        sp.push_back(spacing.x());
        sp.push_back(spacing.y());
        sp.push_back(spacing.z());

        G4VoxelData* data = new G4VoxelData(sh, sp, sizeof(T));
        Init(data);
        
        this->array = reinterpret_cast<std::vector<T>*>(data->array);
        this->array->assign(data->length,(T) 0);
    }

    ~G4VoxelArray() {};

    using G4VoxelArrayBase<T>::GetIndex; 
  
    virtual void Read(G4String, G4String) {};
    virtual void Write(G4String, G4String) {};

    virtual void SetData(G4VoxelData* data) {
        Init(data);
        
        this->array = reinterpret_cast<std::vector<T>*>(data->array);
    };

    virtual void SetValue(T value, unsigned int x, unsigned int y, unsigned int z) {
        unsigned int index = GetIndex(x, y, z);
        (*array)[index] = value;
    }

    virtual void SetValue(T value, G4ThreeVector position) {
        SetValue(value, position.x(), position.y(), position.z());
    };

    virtual void IncrementValue(T value, unsigned int x, unsigned int y, unsigned int z) {
        unsigned int index = GetIndex(x, y, z);
        (*array)[index] += value;
    }
 
    virtual void DecrementValue(T value, unsigned int x, unsigned int y, unsigned int z) {
        IncrementValue(-value, x, y, z);
    }
    
    virtual T GetValue(unsigned int x) {
        unsigned int index = GetIndex(x);
        return (*array)[index]; 
    };

    virtual T GetValue(unsigned int x, unsigned int y) {
        unsigned int index = GetIndex(x, y);
        return GetValue(index);
    };

    virtual T GetValue(unsigned int x, unsigned int y, unsigned int z) {
        unsigned int index = GetIndex(x, y, z);
        return GetValue(index);
    };

    virtual T GetValue(G4ThreeVector position) {
        return GetValue((unsigned int) position.x(),
                (unsigned int) position.y(),
                (unsigned int) position.z());
    }

    T RoundValue(T val, T rounder) {
        if (val > 0) {
            val = floor((val + rounder/2)/rounder)*rounder;
        } else {
            val = floor((val - rounder/2)/rounder)*rounder;
        }

        return val;
    };

    T RoundValue(T val, T lower, T upper, T rounder) {
        val = RoundValue(val, rounder);

        if (val < lower) val = lower;
        if (val > upper) val = upper;

        return val;
    };

    T GetRoundedValue(unsigned int x, T rounder)
    {
        T val = GetValue(x);
        return RoundValue(val, rounder);
    };
    
    T GetRoundedValue(unsigned int x, T lower, T upper, T rounder)
    {
        T val = GetValue(x);
        return RoundValue(val, lower, upper, rounder);
    };

    T GetMaxValue() {
        return *std::max_element(array->begin(), array->end());
    };

    T GetMinValue() {
        return *std::min_element(array->begin(), array->end());
    };

  public:
    std::vector<T>* array;
};


template <typename T>
class G4VoxelArray<std::complex<T> > : public G4VoxelArrayBase<T> {
  public:
    using G4VoxelArrayBase<T>::Init;

    G4VoxelArray(G4VoxelData* data) {
        Init(data);
        
        this->array = reinterpret_cast<std::vector<std::complex<T> >*>(data->array);
    };

    ~G4VoxelArray() {};

    using G4VoxelArrayBase<T>::GetIndex; 
    
    T GetValue(unsigned int x) {
        unsigned int index = GetIndex(x);
        return ((*array)[index]).real(); 
    };

    T GetValue(unsigned int x, unsigned int y) {
        unsigned int index = GetIndex(x, y);
        return ((*array)[index]).real(); 
    }

    T GetRoundedValue(unsigned int x, std::complex<T> rounder)
    {
        T val = GetValue(x);
        T rounder_ = rounder.real();

        if (val < 0) {
            val = floor((val - rounder_/2)/rounder_)*rounder_;
        } else {
            val = floor((val + rounder_/2)/rounder_)*rounder_;
        }

        return val;
    };
    
    T GetRoundedValue(unsigned int x, std::complex<T> lower, std::complex<T> upper,
            std::complex<T> rounder)
    {
        T val = GetRoundedValue(x, rounder);
    
        if (val < lower.real()) val = lower.real();
        if (val > upper.real()) val = upper.real();

        return val; 
    };

  public:
    std::vector<std::complex<T> >* array;
};


#endif // G4VOXELARRAY_H

