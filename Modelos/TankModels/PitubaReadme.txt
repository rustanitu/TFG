File format:

On the first line:
NI NJ NK NUMACTIVE NUMVERTICES NUMSTEPS:
- number of cells in i, j, k (integers)
- number of active cells (integer)
- number of vertices (integer)
- number of recorded time steps (integer)

Follows NUMSTEPS lines, with the format:
dd/mm/year

Follows NUMVERTICES lines, with the format:
X Y Z
- x, y and z coordinates of vertex 'i' (float)

Follows NI*NJ*NK lines, with the format:
ACTIVE I J K N1 N2 N3 N4 N5 N6 N7 N8 AD1 AD2 AD3 AD4 AD5 AD6 PROPS
- ACTIVE: 1 if cell is active, 0 otherwise
- I J and K: i, j and k indices of cell (integer, base 0)
- Ni: index of the 'i'th cell vertex (integers, base 0)
- ADi: index of the cell adjacent by face 'i' of the cell hexahedron, if there is any. -1 if no adjacent cell (integers, base 0)
- PROPS: scalar field value at time step 't' (NUMSTEPS floats)