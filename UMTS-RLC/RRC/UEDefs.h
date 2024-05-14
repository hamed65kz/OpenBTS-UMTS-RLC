#ifndef UEDEFS_H
#define UEDEFS_H
#define CASENAME(x) case x: return #x;
struct UEDefs
{
    enum TransType {
        ttComplete,				// Transaction never used, or Transaction complete.
        ttRrcConnectionSetup,
        ttRrcConnectionRelease,
        ttRadioBearerSetup,
        ttRadioBearerRelease,
        ttCellUpdateConfirm,
        ttSecurityModeCommand
    };
	const char* TransType2Name(TransType ttype)
	{
		switch (ttype) {
			CASENAME(ttComplete)
				CASENAME(ttRrcConnectionSetup)
				CASENAME(ttRrcConnectionRelease)
				CASENAME(ttRadioBearerSetup)
				CASENAME(ttRadioBearerRelease)
				CASENAME(ttCellUpdateConfirm)
				CASENAME(ttSecurityModeCommand)
				// Do not add a default case here; we want the error message if this
				// function gets out of phase with the enumeration.
		}
		return "unrecognized";
	}

    // 4 because the transactionId is only 2 bits.
    static const unsigned sMaxTransaction = 4;
};

#endif // UEDEFS_H
