select city.name, avg(CP.level)
from (
  select C.name, T.id
  from City as C, Trainer as T
  where C.name = T.hometown
  ) as city, CatchedPokemon as CP
where city.id = CP.owner_id
group by city.name
order by avg(CP.level)
