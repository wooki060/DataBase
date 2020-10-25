select P.name, C.level, C.nickname
from CatchedPokemon as C, Pokemon as P
where C.owner_id in(
  select leader_id
  from Gym
  )
and C.nickname like 'A%'
and C.pid = P.id
order by P.name desc
