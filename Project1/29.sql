select count(T.type)
from (
  select type
  from CatchedPokemon as C, Pokemon as P
  where C.pid = P.id
  ) as T
group by T.type
order by T.type
