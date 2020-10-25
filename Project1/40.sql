select hometown, nickname
from (
  select hometown, level, nickname
  from Trainer as T, CatchedPokemon as C
  where T.id = C.owner_id
  order by T.hometown, C.level desc
  ) as hln
group by hometown;
