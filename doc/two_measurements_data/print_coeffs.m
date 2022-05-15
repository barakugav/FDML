function [] = print_coeffs(fid, sol)
  syms d1 d2 k real positive
  syms m1 real positive

  [mnum mden] = numden(sol.A1(1))
  [m n] = size(mnum)
  den = 1
  % for i=1:m
  %   den = den * mden(i)
  % end

  A1 = simplify(expand(reduceRedundancies([sol.A1], [d1, d2, k])))
  print_coeff(fid, 'A1', A1, den)

  B1 = simplify(expand(reduceRedundancies([sol.B1], [d1, d2, k])))
  print_coeff(fid, 'B1', B1, den)

  C1 = simplify(expand(reduceRedundancies([sol.C1], [d1, d2, k])))
  print_coeff(fid, 'C1', C1, den)

  D1 = simplify(expand(reduceRedundancies([sol.D1], [d1, d2, k])))
  print_coeff(fid, 'D1', D1, den)

  fprintf(fid, '\n')
  [mnum mden] = numden(sol.A2(1))
  [m n] = size(mnum)
  den = 1
  % for i=1:m
  %   den = den * mden(i)
  % end

  A2 = simplify(expand(reduceRedundancies([sol.A2], [d1, d2, k])))
  print_coeff(fid, 'A2', A2, den)

  B2 = simplify(expand(reduceRedundancies([sol.B2], [d1, d2, k, m1])))
  print_coeff(fid, 'B2', B2, den)

  C2 = simplify(expand(reduceRedundancies([sol.C2], [d1, d2, k, m1])))
  print_coeff(fid, 'C2', C2, den)

  D2 = simplify(expand(reduceRedundancies([sol.D2], [d1, d2, k, m1])))
  print_coeff(fid, 'D2', D2, den)

%  fprintf(fid, '\n')

end

function [] = print_coeff(fid, name, cf, den)
  fprintf(fid, '%s = %s\n', name, char(simplify((cf(1)*den))))
end
