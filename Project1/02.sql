select name
from Pokemon
where type in (
  select p.type
  from Pokemon AS p
  group by p.type
  having count(p.type) >= (
    select count(type)
    from Pokemon
    group by type
    order by count(type) desc
    limit 1,1)
  )
order by name;
