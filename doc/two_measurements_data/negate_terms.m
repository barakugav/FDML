function [p1, p2, p3, p4, p5, p6, p7, p8, p9] = negate_terms(p1, p2, p3, p4, p5, p6, p7, p8, p9)
  % If the term that multiplies d1^4 is negative, negate all.
  % This guarantees some consistency.

  syms d1 real

  negate = false
  m = size(p1)
  for i=1:m
    if has(p1(i), d1^4)
      if isAlways(p1(i) <= 0)
        negate = true
      end
      break
    end
  end

  if negate
    p1 = -p1	% x^4
    p2 = -p2	% x^3*y
    p3 = -p3  	% x^2*y^2
    p4 = -p4	% x^2
    p5 = -p5    % x*y^3
    p6 = -p6    % x*y
    p7 = -p7    % y^4
    p8 = -p8    % y^2
    p9 = -p9    % free
  end
end
