#pragma once

// 定义 corotational 模型计算弹性能量的方法
#define ENERGY_FROM_STIFFNESSMAT	1	// 结果良好
#define ENERGY_FROM_MODEL			2	// 结果极不稳定，原因未知
#define ENERGY_METHOD				ENERGY_FROM_STIFFNESSMAT

class ModelWrapper
{
public:
	virtual ~ModelWrapper(){}
	virtual double	computeElasticEnergy(const double* u) = 0;
	// 计算 A = [T*f1, T*f2, ..., T*fn]
	// 其中 T 是参数指定的降维矩阵， 尺寸为低维数x有限元网格自由度数
	// f1 ... fn 是各个有限元四面体对内力的贡献
	virtual void	computeReducedForceMatrix(	const double * vertexDisplacements, const EigDense& T, EigDense& A) = 0;
	
	// 计算 A = [RH1, RH2, ..., RHn]
	// 其中 RHi 为 R * Hi * R^T 后，结果按列优先排成一向量
	// Hi为每个元素对切向刚度矩阵的贡献
	// 需要以stiffnessMatrixBuffer参数的形式预先给定刚度矩阵的非零元分布
	virtual void	computeReducedHessianMatrix(const double * vertexDisplacements, const EigDense& R, EigDense& A, SparseMatrix * stiffnessMatrixBuffer)=0;

	virtual bool	setElementMaterialFactor(EigVec& factor)=0;
	virtual double	getElementMaterialFactor(int ithElement)=0;
	virtual void	clearElementMaterialFactor()=0;
};

class CorotationalLinearFEMWrapper: public CorotationalLinearFEM, public ModelWrapper
{
public:
	CorotationalLinearFEMWrapper(TetMesh * tetMesh, int wrap = 2):m_wrap(wrap),CorotationalLinearFEM(tetMesh)
	{
	}

	~CorotationalLinearFEMWrapper(void);

	bool	setElementMaterialFactor(EigVec& factor);
	double	getElementMaterialFactor(int ithElement);
	void	clearElementMaterialFactor(){m_eleMatFactor = EigVec();}
	void   setWrap(int wrap){wrap = wrap;}

	virtual double computeElasticEnergy(const double* u);
	virtual void   ComputeForceAndStiffnessMatrix(double * u, double * f, SparseMatrix * stiffnessMatrix, int warp);

	// 计算 A = [T*f1, T*f2, ..., T*fn]
	// 其中 T 是参数指定的降维矩阵， 尺寸为低维数x有限元网格自由度数
	// f1 ... fn 是各个有限元四面体对内力的贡献
	virtual void computeReducedForceMatrix(	const double * vertexDisplacements, 
											const EigDense& T, 
											EigDense& A);

	virtual void computeReducedHessianMatrix(	const double * vertexDisplacements, 
												const EigDense& reduceMat, 
												EigDense& reducedEleHessianMat, 
												SparseMatrix * stiffnessMatrixBuffer);

private:
	void ComputeForceAndStiffnessMatrixOfSubmesh(double * u, double * f, SparseMatrix * stiffnessMatrix, int warp, int elementLo, int elementHi);

	int				m_wrap;
	EigVec			m_eleMatFactor;			// 缩放每个四面体元素的硬度
};
