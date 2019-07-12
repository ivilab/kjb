#!/usr/bin/env python

# $Id: ps_hun.py 13647 2013-01-24 04:56:33Z predoehl $

'''
This module performs minimum-cost bipartite matching for complete bipartite
graphs with weighted edges, when the cost is based on the sum of the matched
edges.  Just provide the function 'hun' with a square "matrix" of weights.
The "matrix" must be a list of rows, each row contains one weight per column.
Each weight must be nonnegative.
'''

import mbm_dp

class Hun:

	'''
	This computes a minimal bipartite matching of complete bipartite graph
	K_n,n where n is the length of the input, and the input represents a
	square matrix of nonnegative costs.  The i-th element represents the
	i-th row of the matrix, and its j-th element should equal the cost of
	the i-j pairing.  This uses Kuhn's Hungarian method, which is a sort of
	max-flow problem adapted to the circumstances.  The code is expressed in
	python by Andrew Predoehl from pseudocode by C. H. Papadimitriou and
	K. Steiglitz, _Combinatorial Algorithms_, chapters 10, 11
	(Figs. 10-3, 11-2), Dover, 1998.
	'''

	def __init__( this, cost_ij ):
		nnn = len( cost_ij )
		for rrr in cost_ij:
			assert len(rrr) == nnn, 'input is not square'
			for ccc in rrr:
				assert 0 <= ccc, 'input contains a negative cost'

		this._cost = [ cost_ij[i][:] for i in range(nnn) ]
		this._mate_r = nnn * [None]
		this._mate_c = nnn * [None]
		this._alpha = nnn * [0]
		this._inf = 1 + max([max( cost_ij[i] ) for i in range(nnn)]) # infinity
		this._beta = [ min( [ cost_ij[ i ][ j ] for i in range( nnn ) ] )
														for j in range( nnn ) ]
	def answer( this ):
		for r in this._cost:
			this._hstage() # perform one stage
		return this._mate_r

	def _hstage( this ):
		nnn = len( this._cost )
		this._label = nnn * [None]		# entries correspond to rows
		this._slack = nnn * [this._inf]	# entries correspond to columns
		this._nhbor = nnn * [None]		# entries correspond to columns
		A = this._build_aux_graph()
		Q = this._build_queue()
		this._search( A, Q )

	def _build_queue( this ):
		Q = set()
		nnn = len( this._cost )
		for iii in range(nnn):
			if this._mate_r[iii] is None:
				if this._exposed[iii] is not None:
					this._augment( iii )
					return set()
				Q.add( iii )
				this._label[ iii ] = None
				this._relax( iii )
		return Q

	def _augment( this, iii, debugdepth = 0 ):
		assert debugdepth < len( this._cost )
		old_i_mate = this._mate_r[ iii ]	# contents will be a column index
		new_i_mate = this._exposed[ iii ]	# also a column index
		this._mate_r[ iii ] = new_i_mate
		this._mate_c[ new_i_mate ] = iii
		if this._label[ iii ] is not None:
			this._exposed[ this._label[iii] ] = old_i_mate
			this._augment( this._label[iii], debugdepth+1 )

	def _search( this, A, Q ):
		nnn = len( this._cost )
		while 0 < len(Q):
			iii = Q.pop()
			for il in range(nnn):
				if this._label[ il ] is None and (iii,il) in A:
					this._label[ il ] = iii
					Q.add( il )
					if this._exposed[il] is not None:
						this._augment( il )
						return
					this._relax( il )
					this._modify( A, Q )

	def _modify( this, A, Q ):
		th = this._inf
		for d in this._slack:
			if 0 < d:
				th = min( th, d )
		th *= 0.5
		nnn = len( this._cost )
		for iii in range(nnn):
			if this._label[ iii ] is None:
				this._alpha[ iii ] -= th
			else:
				this._alpha[ iii ] += th
		for jjj in range(nnn):
			if 0 == this._slack[ jjj ]:
				this._beta[ jjj ] -= th
			else:
				this._beta[ jjj ] += th
		for jjj in range(jjj):
			if 0 < this._slack[ jjj ]:
				this._slack[ jjj ] -= 2*th
				if 0 == this._slack[ iii ]:
					im = this._mate_c[ jjj ]
					ni = this._nhbor[ jjj ]
					if im is None:
						this._exposed[ ni ] = jjj
						this._augment( ni )
						return
					else:
						this._label[ im ] = ni
						Q.add( im )
						A.add( (ni,im) )

	def _relax( this, iii ):
		for jjj in range( len( this._cost[0] ) ):
			d = this._cost[ iii ][ jjj ] -this._alpha[ iii ] -this._beta[ jjj ]
			if 0 < d < this._slack[ jjj ]:
				this._slack[ jjj ] = d
				this._nhbor[ jjj ] = iii

	def _build_aux_graph( this ):
		A = set()
		nnn = len( this._cost )
		this._exposed = nnn * [None]	# index by row; entries corr. to cols
		for iii in range(nnn):
			for jjj in range(nnn):
				if this._alpha[iii] + this._beta[jjj] == this._cost[iii][jjj]:
					mate = this._mate_c[ jjj ] # mate is a row index
					if mate is None:
						this._exposed[iii] = jjj
					elif iii != mate:
						A.add( (iii,mate) )
		return A

def hun( cost_ij ):
	h = Hun( cost_ij )
	return h.answer()


if __name__ == '__main__':

	import sys

	cij = []
	argc = len(sys.argv);
	if 1 == argc:
		cij = [	[ 23, 42, 17, 93, 20, 17 ],
				[ 87, 42, 23, 23, 90, 10 ],
				[ 12, 23, 34, 45, 56, 67 ],
				[ 87, 76, 15, 54, 43, 32 ],
				[ 82, 64, 93, 11, 44, 10 ],
				[ 23, 23, 23, 23, 23, 23 ]	]
	else:
		import math
		SIZE = int(math.sqrt(argc-1))
		cij = SIZE * [None]
		k = 1
		for i in range(SIZE):
			cij[i] = SIZE * [None]
			for j in range(SIZE):
				cij[i][j] = int(sys.argv[ k ])
				k += 1

	y = hun( cij )
	zl = mbm_dp.mbm( cij )
	zs = mbm_dp.thinner( zl )
	if zs == y:
		print 'success'
	else:
		print 'Hungarian solution ', y, ' and DP solution ', zl, ' differ.'
		ch = mbm_dp.cost_thin( y )
		cdp = mbm_dp.cost_thin( zs )
		if ch == cdp:
			print 'But they have the same cost of', ch
		else:
			print 'Failure.  Hungarian cost =', ch, ',  DP cost =', cdp

