//DirtSampler


inline int intCompare(const void *a, const void *b) {
	const int &ia=*(const int*) a;
	const int &ib=*(const int*) b;
	return ia-ib;
}

struct stExcludeIDList
{
	stExcludeIDList(){};
	~stExcludeIDList()
	{
		IDList.freeMem();
	}

private:
	VR::Table<int>	IDList;
	int				excludeType;
public:
	void Add(int ID){IDList+= ID;}
	void Init(int iCount, int iType = 0)//"exclude" default type 
	{
		IDList.setCount(iCount);
		IDList.setCount(0);
		excludeType = iType;
	}
	void Sort()
	{	
		qsort(&IDList[0], IDList.count(), sizeof(int), intCompare);
	}

	int Find(int renderID) 
	{
		if (IDList.count()==0) return false;

		int b[2];
		for (b[0]=-1, b[1]=IDList.count(); b[1]-b[0]>1;) {
			int middle=(b[0]+b[1])/2;
			b[renderID<IDList[middle]]=middle;
		}
		if (excludeType==0) return (renderID==IDList[b[0]]);
		else return !(renderID==IDList[b[0]]);
	}

	bool Empty() const {
		return IDList.count() == 0;
	}
};


// The occlusion sampler.
struct DirtSampler: VR::AdaptiveColorSampler 
{
private:
	float radius, distribution, falloff;
	int sameObjectOnly, ignoreSelfOcclusion;
	float cosMult;
	int workWithTransparency;
	VR::Vector startPoint;
	bool bAllTranspLevels;
	int mode, sampleEnvironment;

	// Indicates if the intersected, but not affecting the result objects are treated as transparent.
	// If not, no further intersections after these objects are processed.
	int excludedObjectsTransparent; 

	float finalOcclusion;

	VR::Color occludedColor, unoccludedColor;
	
	VR::Matrix nm; // A matrix for transforming from local surface space to world space.

	stExcludeIDList &renderIDs;
	stExcludeIDList &affectresultIDs;
	int affectInclusive;
public:
	DirtSampler(stExcludeIDList &exclList, stExcludeIDList &affectList, int affectListInclusive = 0): 
		renderIDs(exclList), affectresultIDs(affectList), affectInclusive(affectListInclusive) {
		if (affectresultIDs.Empty())
			affectInclusive = 0;
	}
	~DirtSampler(){}

	void init(const VR::VRayContext &rc, VR::Vector &normal, float rad, float distr, int sameOnly, float fall, float csm, 
		int workWithTransparency, const VR::Vector &p, const VR::Color &_occludedColor, const VR::Color &_unoccludedColor,
		bool AllTranspLevels = true, int _ignoreSelfOcclusion=false, int _mode=0, int _sampleEnvironment=false, int _excludedObjectsTransparent=false)
	{
		radius=rad;
		distribution=distr;
		sameObjectOnly=sameOnly;
		falloff=fall;
		mode=_mode;
		if (_mode!=1) {
			VR::makeNormalMatrix(normal, nm);
		} else {
			VR::Vector reflectDir=VR::getReflectDir(rc.rayparams.viewDir, normal);
			VR::makeNormalMatrix(reflectDir, nm);
		}
		cosMult=csm;
		this->workWithTransparency=workWithTransparency;
		startPoint=p;
		bAllTranspLevels = AllTranspLevels;
		ignoreSelfOcclusion = _ignoreSelfOcclusion;
		occludedColor=_occludedColor;
		unoccludedColor=_unoccludedColor;
		sampleEnvironment=_sampleEnvironment;
		excludedObjectsTransparent=_excludedObjectsTransparent;
		startPoint+=nm[2]*rc.vray->getSequenceData().params.options.ray_bias;

		finalOcclusion=0.0f;
	}

	VR::Vector getSpecularDir(float u, float v, float n) {
		float thetaSin;
		if (n>=0.0f) {
			thetaSin=powf(u, 1.0f/(n+1.0f));
		} else {
			thetaSin=1.0f-powf(1.0f-u, 1.0f/(1.0f-n)); 
		}
		float thetaCos=sqrtf(VR::Max(0.0f, 1.0f-thetaSin*thetaSin));
		float phi=2.0f*VR::pi()*v;
		return VR::Vector(cosf(phi)*thetaCos, sinf(phi)*thetaCos, thetaSin);
	}

	VR::Color sampleColor(const VR::VRayContext &rc, VR::VRayContext &nrc, float uc, VR::ValidType &valid) 
	{
		// Compute a sampling direction.
		VR::Vector dir;
		if (mode==0) {
			// Ambient occlusion
			dir=VR::normalize0(nm*this->getSpecularDir(uc, getDMCParam(nrc, 1), distribution));
		} else if (distribution<0.0f) {
			// Mirror
			dir=VR::getReflectDir(rc.rayparams.viewDir, rc.rayresult.normal);
		} else {
			// Glossy AO
			switch (mode) {
				case 1:
					// Ambient occlusion/Phong reflection
					dir=VR::normalize0(nm*VR::getSpecularDir(uc, getDMCParam(nrc, 1), distribution));
					break;
				case 2: {
					// Blinn reflection
					VR::Vector nrm=VR::normalize0(nm*VR::getSpecularDir(uc, getDMCParam(nrc, 1), distribution));
					dir=VR::getReflectDir(rc.rayparams.viewDir, nrm);
					break;
				}
				case 3: {
					// Ward reflection
					float k=-logf(1.0f-uc);
					if (k<0.0f) k=0.0f;

					float roughness=1.0f/(distribution+1.0f);
					VR::real thetaCos=sqrtf(1.0f/(roughness*k+1.0f));
					VR::Vector hn=VR::getSphereDir(thetaCos, getDMCParam(nrc, 1));
					VR::Vector nrm=VR::normalize0(nm*hn);

					dir=VR::getReflectDir(rc.rayparams.viewDir, nrm);
					break;
				}
			}
		}

		// Check if the direction is below the geometric normal - if yes, do nothing.
		float n=dotf(dir, rc.rayresult.gnormal)*cosMult;
		if (n<0.0f) { valid=false; return Vlado::Color(0,0,0); }

		// Set up the ray context for tracing in the given direction.		
		nrc.rayparams.viewDir=dir;
		nrc.rayparams.tracedRay.p=nrc.rayparams.rayOrigin=startPoint;
		nrc.rayparams.tracedRay.dir=dir;
		nrc.rayparams.skipTag=rc.rayresult.skipTag;
		nrc.rayparams.mint=0.0f;
		nrc.rayparams.maxt=radius;

		float occlusion=0.0f;
		float transp=1.0f;
		float step=0.0f;
		bool bGetTexture = false;

		int maxLevels=bAllTranspLevels ? nrc.vray->getSequenceData().params.options.mtl_transpMaxLevels : 1;
		for (int i=0; i<maxLevels; i++) 
		{			
			// Find the first intersection without shading it.
			VR::IntersectionData isData;
			int res=nrc.vray->findIntersection(nrc, &isData);
			if (bAllTranspLevels && res && fabs(isData.wpointCoeff-nrc.rayparams.mint)<1e-6)
			{
				step+=1e-6f+step;
				nrc.rayparams.mint+=step;
				continue;
			}
			step=0.0f;

			// When using TexDirt with VRayScatter with overriden getSurfaceRenderID()
			// there was a crash with incorrect rayresult.primitive pointer
			//
			nrc.rayresult.primitive = isData.primitive;

			// If we have intersected something, increase the occlusion, otherwise - keep it the same.
			if (res) 
			{
				float currentTransp=0.0f;
				float k=0.0f;

				// Indicates if the intersected object is excluded from the calculations
				bool excluded=false;

				VR::DefaultVRayShadeData *sd=(VR::DefaultVRayShadeData*) GET_INTERFACE(isData.sd, EXT_DEFAULT_SHADE_DATA);
				VR::DefaultVRayShadeData *sd2=(VR::DefaultVRayShadeData*) GET_INTERFACE(rc.rayresult.sd, EXT_DEFAULT_SHADE_DATA);

				if (((!sameObjectOnly && (sd && !(affectInclusive ^ affectresultIDs.Find(sd->getSurfaceRenderID(nrc)) ))) || 
					(sameObjectOnly && isData.sb==rc.rayresult.sb)) &&
					(!ignoreSelfOcclusion || isData.sb!=rc.rayresult.sb) &&
					(sd2 && !renderIDs.Find(sd2->getSurfaceRenderID(nrc))) &&
					(!isData.surfaceProps || !isData.surfaceProps->getFlag(VR::surfPropFlag_excludeInAO) ))
					k=(falloff>1e-6f)? powf(1.0f-(float)isData.wpointCoeff/radius, falloff) : 1.0f;
				else excluded=true;

				// Evaluate the transparency of the material
				if (workWithTransparency || (excluded && excludedObjectsTransparent)) {
					if (isData.wpointCoeff>=radius) break;

					nrc.setRayResult(res, &isData);
					if (excluded && excludedObjectsTransparent) {
						// Treat excluded objects as completely transparent
						currentTransp=1.0f;
					}
					else if (nrc.rayresult.sb) {
						nrc.mtlresult.clear();
						nrc.rayresult.sb->shade(nrc);
						currentTransp=nrc.mtlresult.transp.intensity();
					}
				}

				occlusion+=k*(1.0f-currentTransp)*transp;

				transp*=currentTransp;
				if (transp<1e-6f) break;
			} 
			else 
			{
				break;
			}

			if (bAllTranspLevels)
			{
				nrc.rayparams.skipTag=nrc.rayresult.skipTag;
				nrc.rayparams.rayOrigin=nrc.rayresult.wpoint;
				nrc.rayparams.mint=nrc.rayresult.wpointCoeff;
			}
		}

		finalOcclusion+=occlusion;

		VR::Color envFilter;
		if (!sampleEnvironment || occlusion>1.0f-1e-6f) envFilter.makeWhite();
		else {
			nrc.rayresult.shadeID=nrc.threadData->getShadeID(); // We need a new shade ID otherwise the env. texture will just return the same result
			nrc.rayresult.normal=-dir; // Needed if we use TexSampler for bent normals
			nrc.vray->computeEnvironment(nrc);
			envFilter=nrc.mtlresult.color;
		}

		envFilter*=(1.0f-occlusion);

		return occludedColor*occlusion+unoccludedColor*envFilter;

		/*
		if (!sampleEnvironment || occlusion>1.0f-1e-6f) {
			rawColor+=VR::Color(1.0f, 1.0f, 1.0f);
			return occludedColor*occlusion+unoccludedColor*(1.0f-occlusion);
		} else {
			nrc.vray->computeEnvironment(nrc);
			rawColor+=nrc.mtlresult.color;
			return occludedColor*occlusion+(unoccludedColor*nrc.mtlresult.color)*(1.0f-occlusion);
		}
		*/
	}

	void multResult(float m) {
		finalOcclusion*=m;
	}

	float getOcclusion(void) { return finalOcclusion; }
};
