#!/usr/bin/env python

# $Id: mbm_dp.py 13647 2013-01-24 04:56:33Z predoehl $

"""
This computes a minimal bipartite matching, using dynamic programming.
Input is a complete bipartite graph with edge weights (costs) given in a
square matrix (represented by a list of lists; all lists have the same length).
All costs must be nonnegative.

This is gonna be slow, not sure how slow.
"""

def expel(L,i):
	"""
	Expel the element at index i from list L, returning a shorter list.
	Index i must be in bounds of L.
	"""
	assert 0 <= i < len(L)
	x = L[0:i]
	x.extend( L[i+1:] )
	return x

class DP_MinBipartMatch:

	def __init__( self, cij ):
		nnn = len( cij )
		for rrr in cij:
			assert len(rrr) == nnn, 'Input is not square'
			for x in rrr:
				assert 0 <= x, 'Input contains an invalid cost ' + str(x)
		self._cost = cij
		self._memo = {}

	def size( self ):
		return len( self._cost )

	def _validate( self, row_rg, col_rg ):
		assert len(row_rg) == len(col_rg), 'error: unsquare'
		nnn = self.size()
		for iii in range(len(row_rg)):
			assert 0 <= row_rg[iii] < nnn, 'out-of-range row ix'
			assert 0 <= col_rg[iii] < nnn, 'out-of-range col ix'
			assert row_rg[iii] != row_rg[iii-1], 'non-unique row ix'
			assert col_rg[iii] != col_rg[iii-1], 'non-unique col ix'

	def solve( self, row_rg, col_rg ):
		"""
		Returns an optimal association between two sets (lists) of row, col
		indices.  Return value is a tuple of the form (c, i1, j1, i2, j2, ...)
		where c is the cost, followed by 1 or more pairs of i,j associations
		in the optimal solution.
		"""
		if 1 == len(row_rg):
			assert 1 == len(col_rg)
			rix = row_rg[0]
			cix = col_rg[0]
			return ( self._cost[rix][cix], rix, cix ) # base case
		row_rg.sort()
		col_rg.sort()
		self._validate( row_rg, col_rg )
		key = row_rg[:]
		key.extend( col_rg )
		key = tuple(key)
		if key in self._memo:
			return self._memo[key]
		sbest = None
		for k in range(len(col_rg)):
			jk = expel(col_rg, k)
			s = self.solve( row_rg[1:], jk )
			c = self._cost[ row_rg[0] ][ col_rg[k] ] + s[0]
			if sbest is None or c < sbest[0]:
				sbest = [ c, row_rg[0], col_rg[k] ]
				sbest.extend( s[1:] )
		assert sbest is not None
		sbest = tuple( sbest )
		self._memo[ key ] = sbest
		return sbest



def max_cost( cij ):
	big = max(cij[0])
	for r in cij:
		big = max(big, max(r))
	return big

def squarify( cij ):
	nr = len(cij)
	nc = len(cij[0])
	if nr == nc:
		return cij
	big = 1 + max_cost(cij)
	if nr < nc:
		csquare = cij[:]
		while len(csquare) < nc:
			csquare.append( nc * [big] )
		return csquare
	assert nr > nc
	csquare = []
	for r in cij:
		r2 = r[:]
		r2.extend( (nr-nc) * [big] )
		csquare.append( r2 )
	return csquare

def mbm( cij ):
	'''
	See DP_MinBipartMatch::solve for the output format, where the row and
	column indices will be all rows, all columns.
	'''
	m = DP_MinBipartMatch( squarify( cij ) )
	nnn = m.size() 
	return m.solve( range(nnn), range(nnn) )


def pretty_mbm_print( kijij, cost=None ):
	if cost:
		for rr in cost:
			for c in rr:
				print c,
			print
	print kijij
	total = 0
	for k in range(len(kijij)/2):
		rix = kijij[1+2*k]
		cix = kijij[2+2*k]
		print 'Pair row ',rix,' with column ', cix,
		if cost and rix < len(cost) and cix < len(cost[0]):
			total += cost[rix][cix]
			print ' at cost ', cost[rix][cix]
		else:
			print
	if total:
		print 'Total cost:', total


def thinner( ls ):
	'''
	Input is a long solution, output is a short solution.  A long solution is
	as described by mbm().  A short solution omits the cost, and only contains
	the column indices of the solution.  In the short solution s, row i is
	paired with column s[i].
	'''
	nnn = len(ls)/2
	ss = nnn * [None]
	for iii in range(nnn):
		ss[iii] = ls[2+2*iii]
	for cix in ss:
	 	assert cix is not None, 'input is not a valid solution'
	return ss

def cost_thin( lt, cij ):
	'''
	Returns the cost of a thin solution.  Precondition: lt must be a
	permutation.  This function does not check for that.
	'''
	nnn = len(cij)
	assert nnn == len(lt), 'cost is incompatible with solution'
	cost = 0
	for iii in range(nnn):
		cost += cij[ iii ][ lt[iii] ]
	return cost

if __name__ == '__main__':
	
	cij = [	[ 23, 42, 17, 93, 20, 17 ],
			[ 87, 42, 23, 23, 90, 10 ],
			[ 12, 23, 34, 45, 56, 67 ],
			[ 87, 76, 15, 54, 43, 32 ],
			[ 82, 64, 93, 11, 44, 10 ],
			[ 23, 23, 23, 23, 23, 23 ]	]

# 	nonsquare
#	cij = [	[ 23, 42, 17, 93, 20, 17 ],
#			[ 87, 42, 23, 23, 90, 10 ],
#			[ 12, 23, 34, 45, 56, 67 ],
#			[ 87, 76, 15, 54, 43, 32 ],
#			[ 82, 64, 93, 11, 44, 10 ],
#			[ 23, 22, 19, 20, 20, 18 ],
#			[ 20, 19, 18, 29, 22, 21 ] ]

	y = mbm( cij )
	pretty_mbm_print( y, cij )
	print 'short solution: ', thinner(y)

